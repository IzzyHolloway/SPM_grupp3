// Fill out your copyright notice in the Description page of Project Settings.

#include "BoatInteractable.h"
#include "CharacterAimi.h"
#include "DialogueManager.h"
#include "Kismet/GameplayStatics.h"

void ABoatInteractable::Interact()
{
	ACharacterAimi* PlayerCharacter = Cast<ACharacterAimi>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!PlayerCharacter)
	{
		return;
	}

	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (PlayerCharacter->HasRequiredItems())
	{
		UE_LOG(LogTemp, Warning, TEXT("Player can board the boat"));

		if (DialogueManager)
		{
			DialogueManager->ShowMessage(SuccessMessage);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Player is missing items"));

		if (DialogueManager)
		{
			DialogueManager->ShowMessage(BlockedMessage);
		}
	}
}