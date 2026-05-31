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

    // Position the grid from code so coats land on the shelves: no gap between cells (pitch is driven
    // by SlotCellHeight), grid pinned to the top of its container, and pushed down by GridTopOffset so
    // the first row's feet sit on the top shelf. Works as long as SlotGrid sits inside a SizeBox.
    SlotGrid->SetSlotPadding(FMargin(0.f));
    if (USizeBoxSlot* GridBoxSlot = Cast<USizeBoxSlot>(SlotGrid->Slot))
    {
        GridBoxSlot->SetHorizontalAlignment(HAlign_Center);
        GridBoxSlot->SetVerticalAlignment(VAlign_Top);
        GridBoxSlot->SetPadding(FMargin(0.f, GridTopOffset, 0.f, 0.f));
    }

    UDataTable* DT = Wardrobe->CoatDataTable;
    const int32 Cols = FMath::Max(1, Wardrobe->GridColumns);
    const FName EquippedID = Wardrobe->EquippedCoatID;
    const int32 HighlightIndex = Wardrobe->SelectedSlotIndex;

    // Outline size is independent of the cell: the frame overflows freely. Cell footprint = SlotSize,
    // row spacing = SlotCellHeight.
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

        // The interactive button holds only the coat icon (sized to SlotSize).
        UImage* IconImg = nullptr;
        if (bHasCoat && Row && Row->CoatIcon)
        {
            IconImg = NewObject<UImage>(SlotBtn);
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
            SlotBtn->SetContent(IconImg);
        }

        // Force the button (and thus the coat's footprint) to exactly SlotSize so it never changes.
        USizeBox* IconBox = NewObject<USizeBox>(this);
        IconBox->SetWidthOverride(SlotSize.X);
        IconBox->SetHeightOverride(SlotSize.Y);
        IconBox->SetContent(SlotBtn);

        // Cell content: icon and (optional) hover frame are SIBLINGS, both bottom-aligned. The icon's
        // position depends only on its own alignment -- showing/hiding the frame never moves it (no
        // jump). The frame overflows upward to surround the coat.
        UOverlay* CellOverlay = NewObject<UOverlay>(this);
        if (UOverlaySlot* IconBoxSlot = CellOverlay->AddChildToOverlay(IconBox))
        {
            IconBoxSlot->SetHorizontalAlignment(HAlign_Center);
            IconBoxSlot->SetVerticalAlignment(VAlign_Bottom);
        }
        if (bHasCoat && HoverFrameTexture && SlotIdx == HighlightIndex)
        {
            UImage* HoverImg = NewObject<UImage>(CellOverlay);
            FSlateBrush HoverBrush;
            HoverBrush.SetResourceObject(HoverFrameTexture);
            HoverBrush.ImageSize = EffectiveHoverSize;
            HoverBrush.DrawAs = ESlateBrushDrawType::Image;
            HoverImg->SetBrush(HoverBrush);
            HoverImg->SetVisibility(ESlateVisibility::HitTestInvisible);
            // Center the frame on the COAT. It's bottom-aligned (bottom at the cell bottom = the coat's
            // feet); a render translation pushes it DOWN by half the size difference so its center lands
            // on the icon's center and it grows symmetrically around the coat as you enlarge it.
            // (Render translation isn't clamped the way negative slot padding is.)
            HoverImg->SetRenderTranslation(FVector2D(
                HoverFrameOffset.X,
                (EffectiveHoverSize.Y - SlotSize.Y) * 0.5f + HoverFrameOffset.Y));
            if (UOverlaySlot* HoverSlot = CellOverlay->AddChildToOverlay(HoverImg))
            {
                HoverSlot->SetHorizontalAlignment(HAlign_Center);
                HoverSlot->SetVerticalAlignment(VAlign_Bottom);
            }
        }

        // Cell = SlotSize wide x SlotCellHeight tall. Row spacing is SlotCellHeight, independent of the
        // outline size, and the coat rests at the cell bottom.
        USizeBox* SizeWrapper = NewObject<USizeBox>(this);
        const float CellWidth = FMath::Max(SlotCellWidth, SlotSize.X);
        SizeWrapper->SetWidthOverride(CellWidth);
        const float CellHeight = FMath::Max(SlotCellHeight, SlotSize.Y);
        SizeWrapper->SetHeightOverride(CellHeight);
        SizeWrapper->SetContent(CellOverlay);

        if (UUniformGridSlot* GridSlot = SlotGrid->AddChildToUniformGrid(SizeWrapper, Pos / Cols, Pos % Cols))
        {
            GridSlot->SetHorizontalAlignment(HAlign_Center);
            GridSlot->SetVerticalAlignment(VAlign_Bottom);
        }
    }
}
