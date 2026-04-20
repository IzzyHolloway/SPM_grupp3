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

	// Add the flag for this pickup so the game remembers it was collected.
	if (!ProgressFlagToAdd.IsNone())
	{
		ProgressionManager->AddFlag(ProgressFlagToAdd);
	}

	bool bAllRequiredFlagsCollected = true;

	// Check whether all required pickup flags are now present.
	for (const FName RequiredFlag : RequiredFlagsForCompletion)
	{
		if (RequiredFlag.IsNone())
		{
			continue;
		}

		if (!ProgressionManager->HasFlag(RequiredFlag))
		{
			bAllRequiredFlagsCollected = false;
			break;
		}
	}

	// If the collection goal is complete, add the next progression flag.
	if (bAllRequiredFlagsCollected && !CompletionFlagToAdd.IsNone())
	{
		ProgressionManager->AddFlag(CompletionFlagToAdd);
	}

	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (DialogueManager)
	{
		if (bAllRequiredFlagsCollected)
		{
			DialogueManager->ShowMessage(FinalPickupMessage);
		}
		else
		{
			DialogueManager->ShowMessage(PickupMessage);
		}
	}

	Destroy();
}
