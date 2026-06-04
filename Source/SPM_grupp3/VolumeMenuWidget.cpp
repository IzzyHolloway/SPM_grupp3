#include "VolumeMenuWidget.h"
#include "Components/Slider.h"
#include "LittleLost_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "InputCoreTypes.h"

void UVolumeMenuWidget::NativeConstruct()
{
    // Make the whole screen focusable so its NativeOnKeyDown receives Left/Right; we drive
    // the slider from there (a focused UMG slider doesn't reliably take gamepad input).
    SetIsFocusable(true);

    Super::NativeConstruct();

    if (!SoundSlider)
    {
        return;
    }

    if (!SoundSlider->OnValueChanged.IsAlreadyBound(this, &UVolumeMenuWidget::OnVolumeChanged))
    {
        SoundSlider->OnValueChanged.AddDynamic(this, &UVolumeMenuWidget::OnVolumeChanged);
    }

    // Show the saved volume and make sure it's actually applied when the screen opens.
    if (ULittleLost_GameInstance* GI = Cast<ULittleLost_GameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        SoundSlider->SetValue(GI->MasterVolume);
        GI->ApplyMasterVolume();
    }
}

UWidget* UVolumeMenuWidget::GetInitialFocusTarget()
{
    // Focus the widget itself (return nullptr) -- NativeOnKeyDown then reads Left/Right and
    // moves the slider in code, so it works with d-pad, stick and arrow keys alike.
    return nullptr;
}

FReply UVolumeMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (SoundSlider)
    {
        const FKey Key = InKeyEvent.GetKey();
        float Delta = 0.0f;

        if (Key == EKeys::Left || Key == EKeys::Gamepad_DPad_Left || Key == EKeys::Gamepad_LeftStick_Left)
        {
            Delta = -KeyStep;
        }
        else if (Key == EKeys::Right || Key == EKeys::Gamepad_DPad_Right || Key == EKeys::Gamepad_LeftStick_Right)
        {
            Delta = KeyStep;
        }

        if (Delta != 0.0f)
        {
            const float NewValue = FMath::Clamp(SoundSlider->GetValue() + Delta, 0.0f, 1.0f);
            SoundSlider->SetValue(NewValue);
            OnVolumeChanged(NewValue); // SetValue() from code doesn't fire OnValueChanged, so apply here
            return FReply::Handled();
        }
    }

    // Anything else (Backspace / B / Esc) is handled by the base class.
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UVolumeMenuWidget::OnVolumeChanged(float Value)
{
    if (ULittleLost_GameInstance* GI = Cast<ULittleLost_GameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        GI->SetMasterVolume(Value);
    }
}

void UVolumeMenuWidget::HandleBack()
{
    const TCHAR* TargetPath = bOpenedFromPause
        ? TEXT("/Game/Blueprints/WBP/Menus/WBP_PauseMenu.WBP_PauseMenu_C")
        : TEXT("/Game/Blueprints/WBP/Menus/WBP_MainMenu.WBP_MainMenu_C");

    if (UClass* MenuClass = LoadClass<UUserWidget>(nullptr, TargetPath))
    {
        OpenMenu(MenuClass);
    }

    RemoveFromParent();
}
