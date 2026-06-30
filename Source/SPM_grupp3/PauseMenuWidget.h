#pragma once

#include "CoreMinimal.h"
#include "MenuWidgetBase.h"
#include "PauseMenuWidget.generated.h"

/**
 * In-game pause menu: Resume / Save & Quit.
 * Button names must match WBP_PauseMenu.
 */
UCLASS()
class SPM_GRUPP3_API UPauseMenuWidget : public UMenuWidgetBase
{
    GENERATED_BODY()

public:
    UPauseMenuWidget(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_ResumeGame;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_SaveQuit;

    // Optional, for the focus/hover highlight (these are the texts inside the buttons).
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> TextBlock_0;   // "Save & Quit"

    // Level loaded by Save & Quit.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
    FName MainMenuLevel = FName(TEXT("L_MainMenu"));

protected:
    virtual void NativeConstruct() override;
    virtual UWidget* GetInitialFocusTarget() override;
    virtual void HandleBack() override; // Backspace / B / Esc -> resume

    void Resume();

    UFUNCTION() void OnResumeClicked();
    UFUNCTION() void OnSaveQuitClicked();
};
