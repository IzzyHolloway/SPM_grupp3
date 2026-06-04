#pragma once

#include "CoreMinimal.h"
#include "MenuWidgetBase.h"
#include "PlayMenuWidget.generated.h"

/**
 * "Play" sub-menu: New Game / Continue / Back.
 * Button names must match WBP_PlayMenu.
 */
UCLASS()
class SPM_GRUPP3_API UPlayMenuWidget : public UMenuWidgetBase
{
    GENERATED_BODY()

public:
    UPlayMenuWidget(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_NewGame;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_Continue;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_Back;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_NewGame;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Continue;

    // Level the "New Game" button loads. Editable in case the name changes again.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
    FName NewGameLevel = FName(TEXT("LVL_ALittleLost_Level1"));

protected:
    virtual void NativeConstruct() override;
    virtual UWidget* GetInitialFocusTarget() override;
    virtual void HandleBack() override; // Backspace / B / Esc -> back to main menu

    UFUNCTION() void OnNewGameClicked();
    UFUNCTION() void OnContinueClicked();
    UFUNCTION() void OnBackClicked();
};
