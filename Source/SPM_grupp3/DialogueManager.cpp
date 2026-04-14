
#include "DialogueManager.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"

ADialogueManager::ADialogueManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADialogueManager::BeginPlay()
{
	Super::BeginPlay();
}

void ADialogueManager::ShowMessage(const FText& Message)
{
	CurrentMessage = Message;

	UE_LOG(LogTemp, Warning, TEXT("Dialogue Message: %s"), *Message.ToString());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.5f,
			FColor::Cyan,
			Message.ToString()
		);
	}

	// Widget connect comes later when Unreal is open again
}

void ADialogueManager::HideMessage()
{
	UE_LOG(LogTemp, Warning, TEXT("Dialogue hidden"));

	// Widget hiding comes later
}