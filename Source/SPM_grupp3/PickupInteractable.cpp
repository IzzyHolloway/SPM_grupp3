// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupInteractable.h"
#include "DialogueManager.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"

void APickupInteractable::Interact()
{
	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		return;
	}

	// Add this pickup's progression flag if one is configured.
	if (!ProgressFlagToAdd.IsNone())
	{
		ProgressionManager->AddFlag(ProgressFlagToAdd);
	}
	
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	// Show a short feedback message when the pickup is collected.
	if (DialogueManager && !PickupMessage.IsEmpty())
	{
		DialogueManager->ShowMessage(PickupMessage);
	}

	Destroy();
}
