#include "WardrobeViewWidget.h"
#include "Wardrobe.h"
#include "WardrobeComponent.h"
#include "WardrobeDataTypes.h"

#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Engine/Texture2D.h"

void UWardrobeSlotButton::HandleHovered()
{
    if (WardrobeRef.IsValid() && SlotIndex != INDEX_NONE)
    {
        // Ignore mouse hover for a short window after gamepad navigation, so the
        // cursor doesn't snap back to wherever the mouse happens to sit when the
        // grid rebuilds after a navigate.
        if (!WardrobeRef->CanMouseHoverChangeSelection()) return;
        WardrobeRef->SetSelectedSlot(SlotIndex);
    }
}

void UWardrobeSlotButton::HandleClicked()
{
    if (WardrobeRef.IsValid() && SlotIndex != INDEX_NONE)
    {
        WardrobeRef->SetSelectedSlot(SlotIndex);
        WardrobeRef->EquipSelected();
    }
}

void UWardrobeViewWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Take focus so key/gamepad events land here (Esc/Delete/B close, etc).
    SetKeyboardFocus();

    if (Wardrobe)
    {
        if (!Wardrobe->OnWardrobeUpdated.IsAlreadyBound(this, &UWardrobeViewWidget::RefreshSlots))
        {
            Wardrobe->OnWardrobeUpdated.AddDynamic(this, &UWardrobeViewWidget::RefreshSlots);
        }
        RefreshSlots();
    }
}

void UWardrobeViewWidget::NativeDestruct()
{
    if (Wardrobe)
    {
        Wardrobe->OnWardrobeUpdated.RemoveDynamic(this, &UWardrobeViewWidget::RefreshSlots);
    }
    Super::NativeDestruct();
}

// Rebuilds the slot grid. Called on construct and every OnWardrobeUpdated broadcast.
// Each slot is a UWardrobeSlotButton containing an Overlay with:
//   - the coat icon (gray-tinted if locked)
//   - the hover frame (if this slot is currently highlighted by mouse/cursor)
//   - the selected frame (if this coat is the one Lumi wears)
void UWardrobeViewWidget::RefreshSlots()
{
    if (!Wardrobe || !SlotGrid) return;

    SlotGrid->ClearChildren();

    UDataTable* DT = Wardrobe->CoatDataTable;
    const int32 Cols = FMath::Max(1, Wardrobe->GridColumns);
    const FName EquippedID = Wardrobe->EquippedCoatID;
    const int32 HighlightIndex = Wardrobe->SelectedSlotIndex;

    // The hover frame can be larger than the icon -- it OVERFLOWS the cell (Overlay doesn't clip)
    // instead of enlarging it. That keeps the outline size independent of the cell footprint and row
    // spacing, so you can make the outline big without spreading the coats off their shelves.
    const FVector2D EffectiveHoverSize = HoverFrameSize.IsNearlyZero() ? SlotSize : HoverFrameSize;

    // Collect unlocked slot indices in DT order. They fill the grid sequentially
    // starting at position 0, so the visible layout is "stable" — picking up a new
    // coat appends it to the next empty cell instead of slotting it into a fixed gap.
    TArray<int32> Unlocked;
    for (int32 i = 0; i < Wardrobe->Slots.Num(); ++i)
    {
        if (Wardrobe->Slots[i].bUnlocked) Unlocked.Add(i);
    }

    // Render Slots.Num() total cells: first N hold unlocked coats, rest are empty.
    for (int32 Pos = 0; Pos < Wardrobe->Slots.Num(); ++Pos)
    {
        const bool bHasCoat = Pos < Unlocked.Num();
        const int32 SlotIdx = bHasCoat ? Unlocked[Pos] : INDEX_NONE;
        const FCoatDetail* Row = (bHasCoat && DT)
            ? DT->FindRow<FCoatDetail>(Wardrobe->Slots[SlotIdx].CoatID, TEXT("Wardrobe Refresh"))
            : nullptr;

        UWardrobeSlotButton* SlotBtn = NewObject<UWardrobeSlotButton>(this);
        SlotBtn->SlotIndex = SlotIdx;
        SlotBtn->WardrobeRef = Wardrobe;

        // Only filled cells are interactive — empty trailing cells can't be hovered or equipped.
        if (bHasCoat)
        {
            SlotBtn->OnHovered.AddDynamic(SlotBtn, &UWardrobeSlotButton::HandleHovered);
            SlotBtn->OnClicked.AddDynamic(SlotBtn, &UWardrobeSlotButton::HandleClicked);
        }
        else
        {
            SlotBtn->SetIsEnabled(false);
        }

        // Hide the button's own background entirely; only the overlay content renders.
        FButtonStyle Style = SlotBtn->GetStyle();
        Style.Normal.DrawAs   = ESlateBrushDrawType::NoDrawType;
        Style.Hovered.DrawAs  = ESlateBrushDrawType::NoDrawType;
        Style.Pressed.DrawAs  = ESlateBrushDrawType::NoDrawType;
        Style.Disabled.DrawAs = ESlateBrushDrawType::NoDrawType;
        SlotBtn->SetStyle(Style);

        UOverlay* Overlay = NewObject<UOverlay>(SlotBtn);

        // Coat icon — only on cells that hold a coat.
        if (bHasCoat && Row && Row->CoatIcon)
        {
            UImage* IconImg = NewObject<UImage>(Overlay);
            FSlateBrush IconBrush;
            IconBrush.SetResourceObject(Row->CoatIcon);
            IconBrush.ImageSize = SlotSize;
            IconBrush.DrawAs = ESlateBrushDrawType::Image;
            IconImg->SetBrush(IconBrush);

            FLinearColor IconColor = FLinearColor::White;
            const FName CoatID = Wardrobe->Slots[SlotIdx].CoatID;
            if (!CoatID.IsNone() && CoatID == EquippedID)
            {
                IconColor.A *= EquippedOpacity;
            }
            IconImg->SetColorAndOpacity(IconColor);

            if (UOverlaySlot* IconSlot = Overlay->AddChildToOverlay(IconImg))
            {
                IconSlot->SetHorizontalAlignment(HAlign_Center);
                IconSlot->SetVerticalAlignment(VAlign_Center);
            }
        }

        // Hover frame on top of the cell that holds the currently highlighted coat.
        if (bHasCoat && HoverFrameTexture && SlotIdx == HighlightIndex)
        {
            UImage* HoverImg = NewObject<UImage>(Overlay);
            FSlateBrush HoverBrush;
            HoverBrush.SetResourceObject(HoverFrameTexture);
            HoverBrush.ImageSize = EffectiveHoverSize;
            HoverBrush.DrawAs = ESlateBrushDrawType::Image;
            HoverImg->SetBrush(HoverBrush);
            if (UOverlaySlot* HoverSlot = Overlay->AddChildToOverlay(HoverImg))
            {
                HoverSlot->SetHorizontalAlignment(HAlign_Center);
                HoverSlot->SetVerticalAlignment(VAlign_Center);
            }
        }

        SlotBtn->SetContent(Overlay);

        // Fixed icon-sized box so the coat's footprint stays constant (= SlotSize) regardless of the
        // hover frame. The frame just overflows this box when selected, so the icon never shifts.
        USizeBox* IconBox = NewObject<USizeBox>(this);
        IconBox->SetWidthOverride(SlotSize.X);
        IconBox->SetHeightOverride(SlotSize.Y);
        IconBox->SetContent(SlotBtn);

        USizeBox* SizeWrapper = NewObject<USizeBox>(this);
        SizeWrapper->SetWidthOverride(SlotSize.X);
        // Cell can be taller than the icon; the extra height is empty space BELOW the coat, so the
        // coat rests on the shelf instead of floating. This drives row spacing, independent of the
        // outline size.
        const float CellHeight = FMath::Max(SlotCellHeight, SlotSize.Y);
        SizeWrapper->SetHeightOverride(CellHeight);
        if (USizeBoxSlot* WrapperSlot = Cast<USizeBoxSlot>(SizeWrapper->SetContent(IconBox)))
        {
            WrapperSlot->SetHorizontalAlignment(HAlign_Center);
            WrapperSlot->SetVerticalAlignment(VAlign_Bottom);
        }

        // Visible position drives the grid cell. Always Slots.Num() total cells.
        if (UUniformGridSlot* GridSlot = SlotGrid->AddChildToUniformGrid(SizeWrapper, Pos / Cols, Pos % Cols))
        {
            GridSlot->SetHorizontalAlignment(HAlign_Center);
            // Bottom-align so each coat rests on the shelf beneath it instead of floating in the
            // middle of its cell. Fine-tune the resting height with the SlotGrid's SlotPadding
            // (Bottom) in the WBP, or by moving/resizing the grid over the shelf image.
            GridSlot->SetVerticalAlignment(VAlign_Bottom);
        }
    }
}
