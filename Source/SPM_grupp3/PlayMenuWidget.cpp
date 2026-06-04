#include "PlayMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "LittleLost_GameInstance.h"
#include "Kismet/GameplayStatics.h"

UPlayMenuWidget::UPlayMenuWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // This menu's text sits on the light paper frame: dark when idle, red when highlighted.
    NormalTextColor    = FLinearColor(0.012983f, 0.014444f, 0.020289f, 1.0f);
    HighlightTextColor = FLinearColor(0.520996f, 0.064803f, 0.025187f, 1.0f);
}

void UPlayMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button_NewGame)  Button_NewGame->OnClicked.AddDynamic(this, &UPlayMenuWidget::OnNewGameClicked);
    if (Button_Continue) Button_Continue->OnClicked.AddDynamic(this, &UPlayMenuWidget::OnContinueClicked);
    if (Button_Back)     Button_Back->OnClicked.AddDynamic(this, &UPlayMenuWidget::OnBackClicked);

    AddHighlightPair(Button_NewGame, Text_NewGame);

    // Continue is only available when a save exists; otherwise grey it out.
    ULittleLost_GameInstance* GI = Cast<ULittleLost_GameInstance>(UGameplayStatics::GetGameInstance(this));
    const bool bHasSave = GI && GI->HasSave();

    if (Button_Continue)
    {
        Button_Continue->SetIsEnabled(bHasSave);
    }
    if (bHasSave)
    {
        AddHighlightPair(Button_Continue, Text_Continue);
    }
    else if (Text_Continue)
    {
        Text_Continue->SetColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f)));
    }
}

UWidget* UPlayMenuWidget::GetInitialFocusTarget()
{
    // Focus Continue if it's usable, otherwise New Game.
    if (Button_Continue && Button_Continue->GetIsEnabled())
    {
        return Button_Continue;
    }
    return Button_NewGame;
}

void UPlayMenuWidget::OnNewGameClicked()
{
    // Remove this menu BEFORE the level travel so no widget pins the old world (avoids the
    // "Fatal World Leaks" crash) and the new level starts clean.
    RemoveFromParent();

    if (ULittleLost_GameInstance* GI = Cast<ULittleLost_GameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        GI->NewGame(NewGameLevel);
    }
}

void UPlayMenuWidget::OnContinueClicked()
{
    RemoveFromParent();

    if (ULittleLost_GameInstance* GI = Cast<ULittleLost_GameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        GI->ContinueGame();
    }
}

void UPlayMenuWidget::OnBackClicked()
{
    if (UClass* C = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Blueprints/WBP/Menus/WBP_MainMenu.WBP_MainMenu_C")))
    {
        OpenMenu(C);
    }
    RemoveFromParent();
}

void UPlayMenuWidget::HandleBack()
{
    OnBackClicked();
}
