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

	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		return;
	}

	if (!CanPickup(ProgressionManager))
	{
		if (DialogueManager && !BlockedPickupMessage.IsEmpty())
		{
			DialogueManager->ShowMessage(BlockedPickupMessage);
		}

		return;
	}

	if (!ProgressFlagToAdd.IsNone())
	{
		ProgressionManager->AddFlag(ProgressFlagToAdd);
	}

	if (DialogueManager && !PickupMessage.IsEmpty())
	{
		DialogueManager->ShowMessage(PickupMessage);
	}
	
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSound,
			GetActorLocation(),
			PickupSoundVolume
		);
	}

	Destroy();
}


bool APickupInteractable::CanPickup(AProgressionManager* ProgressionManager) const
{
	if (!ProgressionManager)
	{
		return false;
	}

	for (const FName& RequiredFlag : RequiredFlagsToPickup)
	{
		if (RequiredFlag.IsNone())
		{
			continue;
		}

		if (!ProgressionManager->HasFlag(RequiredFlag))
		{
			return false;
		}
	}

	return true;
}