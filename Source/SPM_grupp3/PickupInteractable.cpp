// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupInteractable.h"
#include "CharacterAimi.h"
#include "DialogueManager.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"

void APickupInteractable::Interact()
{
	ACharacterAimi* PlayerCharacter = Cast<ACharacterAimi>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
	);

	if (!PlayerCharacter)
	{
		return;
	}

	PlayerCharacter->AddCollectedItem(1);

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (ProgressionManager && !ProgressFlagToAdd.IsNone())
	{
		ProgressionManager->AddFlag(ProgressFlagToAdd);
	}

	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (DialogueManager)
	{
		if (PlayerCharacter->HasRequiredItems())
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