#pragma once

#include "CoreMinimal.h"
#include "MenuWidgetBase.h"
#include "MainMenuWidget.generated.h"

/**
 * Main menu. Play / Settings / Controls / Credits / Exit.
 * Button names below must match the widgets in WBP_MainMenu.
 */
UCLASS()
class SPM_GRUPP3_API UMainMenuWidget : public UMenuWidgetBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_Play;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_Settings;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_Controls;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_Credits;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_Exit;

    // Optional: only used for the focus/hover text-colour highlight.
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Play;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Settings;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Controls;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Credits;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Exit;

protected:
    virtual void NativeConstruct() override;
    virtual UWidget* GetInitialFocusTarget() override;
    virtual void HandleBack() override; // root menu: ignore back/cancel

    UFUNCTION() void OnPlayClicked();
    UFUNCTION() void OnSettingsClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnCreditsClicked();
    UFUNCTION() void OnExitClicked();
};
