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

    // Opacity multiplier applied to the icon of the currently equipped coat (0-1).
    // 1.0 = full brightness, 0.5 = half dim. Lets the equipped coat read as "you have this one on".
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Visuals", meta=(ClampMin="0.0", ClampMax="1.0"))
    float EquippedOpacity = 0.5f;

    // Forced size of each slot in the grid. Lets icons render at intended pixel size
    // even if the UniformGridPanel cells would otherwise shrink them.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Visuals")
    FVector2D SlotSize = FVector2D(150.f, 150.f);

    // Size of the hover outline. Usually slightly larger than SlotSize so the frame
    // surrounds the icon. Set to (0,0) to fall back to SlotSize.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Visuals")
    FVector2D HoverFrameSize = FVector2D(190.f, 190.f);

    UFUNCTION(BlueprintCallable, Category = "Wardrobe")
    void RefreshSlots();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
};
