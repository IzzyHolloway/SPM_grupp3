#include "PauseMenuWidget.h"
#include "VolumeMenuWidget.h"
#include "LittleLost_GameInstance.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"

UPauseMenuWidget::UPauseMenuWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    NormalTextColor    = FLinearColor(0.012983f, 0.014444f, 0.020289f, 1.0f);
    HighlightTextColor = FLinearColor(0.520996f, 0.064803f, 0.025187f, 1.0f);
}

void UPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button_ResumeGame) Button_ResumeGame->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
    if (SettingsButton)    SettingsButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSettingsClicked);
    if (Button_SaveQuit)   Button_SaveQuit->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSaveQuitClicked);

    AddHighlightPair(SettingsButton, TextBlock_122);
    AddHighlightPair(Button_SaveQuit, TextBlock_0);

    // The Resume/close button sits apart from Settings/Save & Quit, so UMG's default spatial
    // navigation can't reach between them. Wire the d-pad order explicitly: Resume <-> Settings
    // <-> Save & Quit. This is why the controller could "select" (A on the focused Resume) but
    // not move -- now Up/Down walks the list.
    LinkVerticalNavigation({ Cast<UWidget>(Button_ResumeGame), Cast<UWidget>(SettingsButton), Cast<UWidget>(Button_SaveQuit) });
}

UWidget* UPauseMenuWidget::GetInitialFocusTarget()
{
    return Button_ResumeGame ? Cast<UWidget>(Button_ResumeGame) : Cast<UWidget>(SettingsButton);
}

void UPauseMenuWidget::HandleBack()
{
    Resume();
}

void UPauseMenuWidget::Resume()
{
    RemoveFromParent();
    UGameplayStatics::SetGamePaused(this, false);

    if (APlayerController* PC = GetOwningPlayer())
    {
        UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
        PC->bShowMouseCursor = false;
    }
}

void UPauseMenuWidget::OnResumeClicked()
{
    Resume();
}

void UPauseMenuWidget::OnSettingsClicked()
{
    // Open the volume screen and tell it to return to the pause menu (not the main menu).
    if (UClass* C = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Blueprints/WBP/Menus/WBP_Volume.WBP_Volume_C")))
    {
        UUserWidget* W = CreateWidget<UUserWidget>(GetOwningPlayer(), C);
        if (UVolumeMenuWidget* Vol = Cast<UVolumeMenuWidget>(W))
        {
            Vol->bOpenedFromPause = true;
        }
        if (W)
        {
            W->AddToViewport();
        }
    }
    RemoveFromParent();
}

void UPauseMenuWidget::OnSaveQuitClicked()
{
    UGameplayStatics::SetGamePaused(this, false);

    if (ULittleLost_GameInstance* GI = Cast<ULittleLost_GameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        GI->SaveGameAsync(); // captures the world synchronously, then writes on a background thread
    }

    RemoveFromParent();
    UGameplayStatics::OpenLevel(this, MainMenuLevel);
}
