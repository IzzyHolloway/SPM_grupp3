#pragma once

#include "CoreMinimal.h"
#include "MenuWidgetBase.h"
#include "MainMenuWidget.generated.h"


UCLASS()
class SPM_GRUPP3_API UMainMenuWidget : public UMenuWidgetBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_Play;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_Controls;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_Credits;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_Exit;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Play;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Controls;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Credits;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Exit;

protected:
    virtual void NativeConstruct() override;
    virtual UWidget* GetInitialFocusTarget() override;
    virtual void HandleBack() override; // root menu: ignore back/cancel

    UFUNCTION() void OnPlayClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnCreditsClicked();
    UFUNCTION() void OnExitClicked();
};
