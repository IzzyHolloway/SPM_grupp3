#include "MenuWidgetBase.h"
#include "Components/Widget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputCoreTypes.h"

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
        // UI-only is what lets the gamepad navigate UMG (move focus, drag the slider).
        UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PC, nullptr, EMouseLockMode::DoNotLock, false);
        PC->bShowMouseCursor = true;
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

    // Backspace (keyboard), B / right face button (gamepad), Escape -> go back.
    if (Key == EKeys::BackSpace || Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Escape)
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
