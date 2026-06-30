#include "PauseMenuWidget.h"
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
    if (Button_SaveQuit)   Button_SaveQuit->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSaveQuitClicked);

    AddHighlightPair(Button_SaveQuit, TextBlock_0);

    // The Resume/close button sits apart from Save & Quit, so UMG's default spatial navigation
    // can't reach between them. Wire the d-pad order explicitly so Up/Down walks the list.
    LinkVerticalNavigation({ Cast<UWidget>(Button_ResumeGame), Cast<UWidget>(Button_SaveQuit) });
}

UWidget* UPauseMenuWidget::GetInitialFocusTarget()
{
    // Open with focus on Save & Quit (falls back to Resume if it's missing).
    return Button_SaveQuit ? Cast<UWidget>(Button_SaveQuit) : Cast<UWidget>(Button_ResumeGame);
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
