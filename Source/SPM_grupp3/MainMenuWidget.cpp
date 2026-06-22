#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
    const TCHAR* PlayMenuPath = TEXT("/Game/Blueprints/WBP/Menus/WBP_PlayMenu.WBP_PlayMenu_C");
    const TCHAR* VolumePath   = TEXT("/Game/Blueprints/WBP/Menus/WBP_Volume.WBP_Volume_C");
    const TCHAR* ControlsPath = TEXT("/Game/Blueprints/WBP/Menus/WBP_Controls.WBP_Controls_C");
    const TCHAR* CreditsPath  = TEXT("/Game/Blueprints/WBP/Menus/WBP_Credits.WBP_Credits_C");
}

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button_Play)     Button_Play->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayClicked);
    if (Button_Settings) Button_Settings->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
    if (Button_Controls) Button_Controls->OnClicked.AddDynamic(this, &UMainMenuWidget::OnControlsClicked);
    if (Button_Credits)  Button_Credits->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreditsClicked);
    if (Button_Exit)     Button_Exit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitClicked);

    AddHighlightPair(Button_Play, Text_Play);
    AddHighlightPair(Button_Settings, Text_Settings);
    AddHighlightPair(Button_Controls, Text_Controls);
    AddHighlightPair(Button_Credits, Text_Credits);
    AddHighlightPair(Button_Exit, Text_Exit);

    // The menu is HitTestInvisible (mouse fully disabled), so UMG's default spatial navigation
    // has no hit-test grid to search -- wire the d-pad order explicitly instead.
    LinkVerticalNavigation({
        Cast<UWidget>(Button_Play),
        Cast<UWidget>(Button_Settings),
        Cast<UWidget>(Button_Controls),
        Cast<UWidget>(Button_Credits),
        Cast<UWidget>(Button_Exit) });
}

UWidget* UMainMenuWidget::GetInitialFocusTarget()
{
    return Button_Play;
}

void UMainMenuWidget::HandleBack()
{
    // This is the root menu -- Backspace/B/Esc should do nothing here.
}

void UMainMenuWidget::OnPlayClicked()
{
    if (UClass* C = LoadClass<UUserWidget>(nullptr, PlayMenuPath)) OpenMenu(C);
    RemoveFromParent();
}

void UMainMenuWidget::OnSettingsClicked()
{
    // Opened from the main menu -> Volume returns here (bOpenedFromPause stays false).
    if (UClass* C = LoadClass<UUserWidget>(nullptr, VolumePath)) OpenMenu(C);
    RemoveFromParent();
}

void UMainMenuWidget::OnControlsClicked()
{
    if (UClass* C = LoadClass<UUserWidget>(nullptr, ControlsPath)) OpenMenu(C);
    RemoveFromParent();
}

void UMainMenuWidget::OnCreditsClicked()
{
    if (UClass* C = LoadClass<UUserWidget>(nullptr, CreditsPath)) OpenMenu(C);
    RemoveFromParent();
}

void UMainMenuWidget::OnExitClicked()
{
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
