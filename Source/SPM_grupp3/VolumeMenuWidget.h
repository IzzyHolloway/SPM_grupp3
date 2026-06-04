#pragma once

#include "CoreMinimal.h"
#include "MenuWidgetBase.h"
#include "VolumeMenuWidget.generated.h"

class USlider;

/**
 * Sound-settings menu. Drives the master volume from the slider (mouse OR controller),
 * stores it on the GameInstance, and returns to the menu it was opened from.
 * The WBP only needs a USlider named "SoundSlider" -- no Blueprint graph.
 */
UCLASS()
class SPM_GRUPP3_API UVolumeMenuWidget : public UMenuWidgetBase
{
    GENERATED_BODY()

public:
    // Set by whoever opens this menu: true = opened from the pause menu (return there),
    // false = opened from the main menu (return there). ExposeOnSpawn so the (temporary)
    // Blueprint "Create Widget" openers can still set it until they're moved to C++ too.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu", meta = (ExposeOnSpawn = "true"))
    bool bOpenedFromPause = false;

    // Must match a USlider named "SoundSlider" in WBP_Volume.
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<USlider> SoundSlider;

    // How much the volume changes per Left/Right press (0..1).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
    float KeyStep = 0.05f;

protected:
    virtual void NativeConstruct() override;
    virtual UWidget* GetInitialFocusTarget() override;
    virtual void HandleBack() override;
    // Left/Right (keyboard arrows, gamepad d-pad, left stick) drive the slider from code.
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    UFUNCTION()
    void OnVolumeChanged(float Value);
};
