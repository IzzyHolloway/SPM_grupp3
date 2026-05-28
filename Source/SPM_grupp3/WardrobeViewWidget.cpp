#include "WardrobeViewWidget.h"
#include "Wardrobe.h"
#include "WardrobeComponent.h"
#include "WardrobeDataTypes.h"

#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Engine/Texture2D.h"

void UWardrobeSlotButton::HandleHovered()
{
    if (WardrobeRef.IsValid() && SlotIndex != INDEX_NONE)
    {
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

    int32 VisibleIndex = 0;
    for (int32 i = 0; i < Wardrobe->Slots.Num(); ++i)
    {
        const FWardrobeSlot& WardrobeSlot = Wardrobe->Slots[i];

        // Skip locked coats entirely unless the widget is configured to show them.
        if (!WardrobeSlot.bUnlocked && !bShowLockedSlots) continue;

        const FCoatDetail* Row = DT
            ? DT->FindRow<FCoatDetail>(WardrobeSlot.CoatID, TEXT("Wardrobe Refresh"))
            : nullptr;

        UWardrobeSlotButton* SlotBtn = NewObject<UWardrobeSlotButton>(this);
        SlotBtn->SlotIndex = i;
        SlotBtn->WardrobeRef = Wardrobe;
        SlotBtn->OnHovered.AddDynamic(SlotBtn, &UWardrobeSlotButton::HandleHovered);
        SlotBtn->OnClicked.AddDynamic(SlotBtn, &UWardrobeSlotButton::HandleClicked);

        // Hide the button's own background; visuals are entirely the overlay.
        FButtonStyle Style = SlotBtn->GetStyle();
        Style.Normal.TintColor   = FSlateColor(FLinearColor::Transparent);
        Style.Hovered.TintColor  = FSlateColor(FLinearColor::Transparent);
        Style.Pressed.TintColor  = FSlateColor(FLinearColor::Transparent);
        Style.Disabled.TintColor = FSlateColor(FLinearColor::Transparent);
        SlotBtn->SetStyle(Style);

        UOverlay* Overlay = NewObject<UOverlay>(SlotBtn);

        // Coat icon. Force the brush size so the icon fills SlotSize regardless of
        // the underlying texture's import resolution.
        UImage* IconImg = NewObject<UImage>(Overlay);
        if (Row && Row->CoatIcon)
        {
            FSlateBrush IconBrush;
            IconBrush.SetResourceObject(Row->CoatIcon);
            IconBrush.ImageSize = SlotSize;
            IconBrush.DrawAs = ESlateBrushDrawType::Image;
            IconImg->SetBrush(IconBrush);
        }
        IconImg->SetColorAndOpacity(WardrobeSlot.bUnlocked ? FLinearColor::White : LockedTint);
        if (UOverlaySlot* IconSlot = Overlay->AddChildToOverlay(IconImg))
        {
            IconSlot->SetHorizontalAlignment(HAlign_Center);
            IconSlot->SetVerticalAlignment(VAlign_Center);
        }

        // Hover frame on top of the highlighted slot.
        if (HoverFrameTexture && i == HighlightIndex)
        {
            UImage* HoverImg = NewObject<UImage>(Overlay);
            HoverImg->SetBrushFromTexture(HoverFrameTexture);
            if (UOverlaySlot* HoverSlot = Overlay->AddChildToOverlay(HoverImg))
            {
                HoverSlot->SetHorizontalAlignment(HAlign_Center);
                HoverSlot->SetVerticalAlignment(VAlign_Center);
            }
        }

        // Selected frame on top of the equipped coat.
        if (SelectedFrameTexture && !WardrobeSlot.CoatID.IsNone() && WardrobeSlot.CoatID == EquippedID)
        {
            UImage* SelectedImg = NewObject<UImage>(Overlay);
            SelectedImg->SetBrushFromTexture(SelectedFrameTexture);
            if (UOverlaySlot* SelectedSlot = Overlay->AddChildToOverlay(SelectedImg))
            {
                SelectedSlot->SetHorizontalAlignment(HAlign_Center);
                SelectedSlot->SetVerticalAlignment(VAlign_Center);
            }
        }

        SlotBtn->SetContent(Overlay);

        // Wrap in SizeBox so each cell renders at the intended pixel size, regardless
        // of how the UniformGridPanel would otherwise distribute available space.
        USizeBox* SizeWrapper = NewObject<USizeBox>(this);
        SizeWrapper->SetWidthOverride(SlotSize.X);
        SizeWrapper->SetHeightOverride(SlotSize.Y);
        SizeWrapper->SetContent(SlotBtn);

        // Pack unlocked coats sequentially using VisibleIndex so the grid has no gaps.
        if (UUniformGridSlot* GridSlot = SlotGrid->AddChildToUniformGrid(SizeWrapper, VisibleIndex / Cols, VisibleIndex % Cols))
        {
            GridSlot->SetHorizontalAlignment(HAlign_Center);
            GridSlot->SetVerticalAlignment(VAlign_Center);
        }
        ++VisibleIndex;
    }
}
