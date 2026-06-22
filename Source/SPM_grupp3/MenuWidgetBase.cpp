#include "MenuWidgetBase.h"
#include "Components/Widget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputCoreTypes.h"
#include "Types/SlateEnums.h"

void UMenuWidgetBase::NativeConstruct()
{
    Super::NativeConstruct();

    SetupMenuInput();
    
    bInitialFocusApplied = false;
}

void UMenuWidgetBase::SetupMenuInput()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        UWidget* FocusTarget = GetInitialFocusTarget();


        UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PC, FocusTarget, EMouseLockMode::DoNotLock, false);

        // Controller-only menus: hide the mouse cursor so the mouse can't drive the menu.
        PC->bShowMouseCursor = false;

        if (FocusTarget)
        {
            FocusTarget->SetKeyboardFocus();
        }
    }
}

UWidget* UMenuWidgetBase::GetInitialFocusTarget()
{
    return nullptr;
}

void UMenuWidgetBase::HandleBack()
{
    RemoveFromParent();
}

FReply UMenuWidgetBase::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    if (Key == EKeys::BackSpace || Key == EKeys::Gamepad_FaceButton_Right
        || Key == EKeys::Gamepad_Special_Right || Key == EKeys::Escape)
    {
        HandleBack();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

UUserWidget* UMenuWidgetBase::OpenMenu(TSubclassOf<UUserWidget> MenuClass)
{
    if (!MenuClass)
    {
        return nullptr;
    }

    UUserWidget* NewMenu = CreateWidget<UUserWidget>(GetOwningPlayer(), MenuClass);
    if (NewMenu)
    {
        NewMenu->AddToViewport();
    }
    return NewMenu;
}

void UMenuWidgetBase::AddHighlightPair(UButton* Button, UTextBlock* Text)
{
    if (Button && Text)
    {
        HighlightButtons.Add(Button);
        HighlightTexts.Add(Text);
    }
}

void UMenuWidgetBase::LinkVerticalNavigation(const TArray<UWidget*>& Widgets)
{
    for (int32 i = 0; i < Widgets.Num(); ++i)
    {
        UWidget* Current = Widgets[i];
        if (!Current)
        {
            continue;
        }
        if (i > 0 && Widgets[i - 1])
        {
            Current->SetNavigationRuleExplicit(EUINavigation::Up, Widgets[i - 1]);
        }
        if (i + 1 < Widgets.Num() && Widgets[i + 1])
        {
            Current->SetNavigationRuleExplicit(EUINavigation::Down, Widgets[i + 1]);
        }
    }
}

void UMenuWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Apply the initial focus once, on the first frame after the widget is live.
    if (!bInitialFocusApplied)
    {
        bInitialFocusApplied = true;
        if (UWidget* FocusTarget = GetInitialFocusTarget())
        {
            FocusTarget->SetKeyboardFocus();
        }
        else
        {
            SetIsFocusable(true);
            SetKeyboardFocus();
        }
    }

    for (int32 i = 0; i < HighlightButtons.Num(); ++i)
    {
        UButton* Button = HighlightButtons[i];
        UTextBlock* Text = HighlightTexts[i];
        if (!Button || !Text)
        {
            continue;
        }

        FLinearColor Colour = NormalTextColor;
        if (Button->IsPressed())
        {
            Colour = PressedTextColor;
        }
        else if (Button->HasAnyUserFocus() || Button->HasKeyboardFocus())
        {
            // Controller-only: highlight follows gamepad/keyboard focus, not the mouse.
            Colour = HighlightTextColor;
        }
        Text->SetColorAndOpacity(FSlateColor(Colour));
    }
}
