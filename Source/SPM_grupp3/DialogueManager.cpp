
#include "DialogueManager.h"
#include "Blueprint/UserWidget.h"
#include "DialogueWidgetBase.h"
#include "Engine/Engine.h"
ADialogueManager::ADialogueManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADialogueManager::BeginPlay()
{
	Super::BeginPlay();

	if (DialogueWidgetClass)
	{
		DialogueWidgetInstance = CreateWidget<UDialogueWidgetBase>(GetWorld(), DialogueWidgetClass);

		if (DialogueWidgetInstance)
		{
			DialogueWidgetInstance->AddToViewport();
			DialogueWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ADialogueManager::ShowMessage(const FText& Message)
{
	CurrentMessage = Message;

	UE_LOG(LogTemp, Warning, TEXT("Dialogue Message: %s"), *Message.ToString());

	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		DialogueWidgetInstance->SetDialogueText(Message);
	}
}

void ADialogueManager::HideMessage()
{
	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}
}