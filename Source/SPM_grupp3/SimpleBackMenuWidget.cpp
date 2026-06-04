#include "SimpleBackMenuWidget.h"
#include "Components/Button.h"

void USimpleBackMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button_Back)
    {
        Button_Back->OnClicked.AddDynamic(this, &USimpleBackMenuWidget::OnBackClicked);
    }
}

UWidget* USimpleBackMenuWidget::GetInitialFocusTarget()
{
    return Button_Back;
}

void USimpleBackMenuWidget::HandleBack()
{
    if (UClass* C = LoadClass<UUserWidget>(nullptr, *ReturnMenuPath))
    {
        OpenMenu(C);
    }
    RemoveFromParent();
}

void USimpleBackMenuWidget::OnBackClicked()
{
    HandleBack();
}
