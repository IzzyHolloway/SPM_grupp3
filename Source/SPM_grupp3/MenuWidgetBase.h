#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuWidgetBase.generated.h"

class UButton;
class UTextBlock;

/**
 * Shared base for all the game's menu widgets (Volume, MainMenu, PlayMenu, ...).
 * Handles, entirely in C++ (no Blueprint logic needed):
 *   - UI-only input mode + mouse cursor on open, so both mouse and controller work
 *   - Initial keyboard/gamepad focus (controller navigation), via GetInitialFocusTarget()
 *   - Backspace / Gamepad-B / Escape -> HandleBack()
 * Subclasses override GetInitialFocusTarget() and HandleBack() as needed.
 */
UCLASS()
class SPM_GRUPP3_API UMenuWidgetBase : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    // Controller is primary: cursor is hidden on open and on gamepad input, shown again on mouse move.
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    void SetMouseCursorVisible(bool bVisible);

    // The widget that should have focus when this menu opens, so a controller can
    // navigate immediately. Return e.g. the first button, or the slider on the volume
    // screen. Default: nullptr -> focus the menu widget itself.
    virtual UWidget* GetInitialFocusTarget();

    // What "back / cancel" does (Backspace, Gamepad B, Escape). Default: close this menu.
    virtual void HandleBack();

    // Put the player in UI-only input mode and show the cursor (mouse + controller).
    void SetupMenuInput();

    // Create + show another menu widget by class. Returns it so callers can configure it.
    UUserWidget* OpenMenu(TSubclassOf<UUserWidget> MenuClass);

    // Per-frame highlight: registered texts turn HighlightTextColor while their button is
    // moused-over OR has controller focus, else NormalTextColor. Mirrors the old
    // Get_Text_..._ColorAndOpacity bindings so both mouse and gamepad highlight identically.
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Register a (button, text) pair to be highlighted. Call from a subclass's NativeConstruct.
    void AddHighlightPair(UButton* Button, UTextBlock* Text);

    // Wire explicit Up/Down gamepad navigation between a vertical list of widgets, so the
    // controller can move between them even when the visual layout defeats UMG's default
    // spatial search (e.g. a close button in a corner + buttons in a centered box).
    void LinkVerticalNavigation(const TArray<UWidget*>& Widgets);

    // Text colours: idle / hovered-or-focused / pressed. Tweakable per widget in the editor.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Style")
    FLinearColor NormalTextColor = FLinearColor(1.0f, 0.871367f, 0.693872f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Style")
    FLinearColor HighlightTextColor = FLinearColor(0.520996f, 0.064803f, 0.025187f, 1.0f); // #BF482C

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Style")
    FLinearColor PressedTextColor = FLinearColor(0.230740f, 0.028426f, 0.010960f, 1.0f); // #842F1B

private:
    // Initial keyboard focus is applied on the first tick (Construct is too early), once.
    bool bInitialFocusApplied = false;

    UPROPERTY()
    TArray<TObjectPtr<UButton>> HighlightButtons;

    UPROPERTY()
    TArray<TObjectPtr<UTextBlock>> HighlightTexts;
};
