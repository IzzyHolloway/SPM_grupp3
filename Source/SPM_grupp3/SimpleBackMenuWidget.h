#pragma once

#include "CoreMinimal.h"
#include "MenuWidgetBase.h"
#include "SimpleBackMenuWidget.generated.h"

/**
 * A menu whose only action is "go back to another menu" -- used by WBP_Controls and
 * WBP_Credits. Reparent both of those to this class. Back button, plus Backspace/B/Esc.
 */
UCLASS()
class SPM_GRUPP3_API USimpleBackMenuWidget : public UMenuWidgetBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> Button_Back;

    // Where "back" goes. Both Controls and Credits return to the main menu.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
    FString ReturnMenuPath = TEXT("/Game/Blueprints/WBP/Menus/WBP_MainMenu.WBP_MainMenu_C");

protected:
    virtual void NativeConstruct() override;
    virtual UWidget* GetInitialFocusTarget() override;
    virtual void HandleBack() override;

    UFUNCTION() void OnBackClicked();
};
