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

    // Focus is applied on the first NativeTick, NOT here: setting keyboard focus during
    // Construct is too early and silently fails (the widget isn't in the focus path yet),
    // which left the gamepad dead until the mouse moved. NativeTick also runs while the game
    // is paused, so this gives the pause menu working controller focus too.
    bInitialFocusApplied = false;
}

void UMenuWidgetBase::SetupMenuInput()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        UWidget* FocusTarget = GetInitialFocusTarget();

        // UI-only lets the gamepad navigate UMG. Passing the focus target here sets focus
        // synchronously as part of establishing the mode -- this is what makes the controller
        // work in the PAUSE menu, where widgets don't tick (so a deferred focus never runs).
        UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PC, FocusTarget, EMouseLockMode::DoNotLock, false);

        // Controller is primary: hide the cursor on open. NativeOnMouseMove shows it again.
        PC->bShowMouseCursor = false;

        if (FocusTarget)
        {
            FocusTarget->SetKeyboardFocus();
        }
    }
}

void UMenuWidgetBase::SetMouseCursorVisible(bool bVisible)
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (PC->bShowMouseCursor != bVisible)
        {
            PC->bShowMouseCursor = bVisible;
        }
    }
}

FReply UMenuWidgetBase::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    SetMouseCursorVisible(true);
    return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
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

    // Any controller input hides the mouse cursor (controller is primary).
    if (Key.IsGamepadKey())
    {
        SetMouseCursorVisible(false);
    }

    // Backspace (keyboard), B / right face button (gamepad), Start/Menu (Special_Right),
    // Escape -> go back. Special_Right keeps "close pause with the Menu button" working
    // after the old Blueprint OnPreviewKeyDown that did it is deleted.
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
        else if (Button->IsHovered() || Button->HasAnyUserFocus() || Button->HasKeyboardFocus())
        {
            Colour = HighlightTextColor;
        }
        Text->SetColorAndOpacity(FSlateColor(Colour));
    }
}
