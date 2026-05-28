#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "WardrobeViewWidget.generated.h"

class UWardrobeComponent;
class AWardrobe;
class UUniformGridPanel;
class UTexture2D;

// Per-slot button. Knows its slot index and routes hover/click into the wardrobe component.
// Created at runtime by UWardrobeViewWidget::RefreshSlots, not placed in the WBP.
UCLASS()
class SPM_GRUPP3_API UWardrobeSlotButton : public UButton
{
    GENERATED_BODY()

public:
    int32 SlotIndex = INDEX_NONE;

    UPROPERTY()
    TWeakObjectPtr<UWardrobeComponent> WardrobeRef;

    UFUNCTION()
    void HandleHovered();

    UFUNCTION()
    void HandleClicked();
};

UCLASS()
class SPM_GRUPP3_API UWardrobeViewWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Wardrobe")
    TObjectPtr<UWardrobeComponent> Wardrobe;

    UPROPERTY(BlueprintReadWrite, Category = "Wardrobe")
    TObjectPtr<AWardrobe> WardrobeActor;

    // Must match a UUniformGridPanel named "SlotGrid" in the WBP.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUniformGridPanel> SlotGrid;

    // Outline shown on the cursor's current slot (hover/highlight). Assign in WBP defaults.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Visuals")
    TObjectPtr<UTexture2D> HoverFrameTexture;

    // Outline shown on the currently equipped coat. Assign in WBP defaults.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Visuals")
    TObjectPtr<UTexture2D> SelectedFrameTexture;

    // Tint applied to a coat icon when its slot is still locked.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Visuals")
    FLinearColor LockedTint = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Wardrobe")
    void RefreshSlots();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
};
