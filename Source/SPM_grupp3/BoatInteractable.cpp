// Fill out your copyright notice in the Description page of Project Settings.
#include "BoatInteractable.h"
#include "DialogueManager.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"

void ABoatInteractable::Interact()
{
	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		return;
	}

	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	const bool bCanUseBoat =
		!RequiredProgressFlag.IsNone() &&
		ProgressionManager->HasFlag(RequiredProgressFlag);

	if (bCanUseBoat)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player can board the boat"));

		if (DialogueManager)
		{
			DialogueManager->ShowMessage(SuccessMessage);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Player is missing required progression"));

		if (DialogueManager)
		{
			DialogueManager->ShowMessage(BlockedMessage);
		}
	}
}
