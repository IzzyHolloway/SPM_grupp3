// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupInteractable.h"
#include "CharacterAimi.h"
#include "DialogueManager.h"
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

	UE_LOG(LogTemp, Warning, TEXT("Pickup collected"));

	Destroy();
}