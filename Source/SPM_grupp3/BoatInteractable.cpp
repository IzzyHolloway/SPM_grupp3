// Fill out your copyright notice in the Description page of Project Settings.
#include "BoatInteractable.h"
#include "DialogueManager.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"

void ABoatInteractable::Interact()
{
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (!DialogueManager || !ProgressionManager)
	{
		return;
	}

	// The boat only allows progression if the required flag is present.
	if (!RequiredProgressFlag.IsNone() && ProgressionManager->HasFlag(RequiredProgressFlag))
	{
		DialogueManager->ShowMessage(SuccessMessage);
	}
	else
	{
		DialogueManager->ShowMessage(BlockedMessage);
	}
}

bool ABoatInteractable::CanUseBoat(AProgressionManager* ProgressionManager) const
{
	if (!ProgressionManager)
	{
		return false;
	}
	
	if (RequiredProgressFlag.IsNone())
	{
		return true;
	}
	return ProgressionManager->HasFlag(RequiredProgressFlag);
}