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

	// Remember that this pickup has been collected.
	if (!ProgressFlagToAdd.IsNone())
	{
		ProgressionManager->AddFlag(ProgressFlagToAdd);
	}

	bool bAllRequiredFlagsCollected = true;

	// Check whether all required progression flags for this milestone are present.
	for (const FName& RequiredFlag : RequiredFlagsForCompletion)
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

	// If all required flags are present, unlock the next progression step
	// and update the current objective if configured.
	if (bAllRequiredFlagsCollected)
	{
		if (!CompletionFlagToAdd.IsNone())
		{
			ProgressionManager->AddFlag(CompletionFlagToAdd);
		}

		if (!ObjectiveTextOnCompletion.IsEmpty())
		{
			ProgressionManager->SetCurrentObjectiveText(ObjectiveTextOnCompletion);
		}

		if (!ObjectiveIDOnCompletion.IsNone())
		{
			ProgressionManager->SetCurrentObjectiveID(ObjectiveIDOnCompletion);
		}
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
