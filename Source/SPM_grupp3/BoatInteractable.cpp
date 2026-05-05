// Fill out your copyright notice in the Description page of Project Settings.

#include "BoatInteractable.h"
#include "ProgressionManager.h"
#include "DialogueManager.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void ABoatInteractable::Interact()
{
	Super::Interact();

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("BoatInteractable: No ProgressionManager found."));
		return;
	}

	ACharacter* PlayerCharacter = GetPlayerCharacter();

	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("BoatInteractable: No player character found."));
		return;
	}

	if (!CanBoard(ProgressionManager))
	{
		if (DialogueManager)
		{
			DialogueManager->ShowMessage(BlockedMessage);
		}

		return;
	}

	if (DialogueManager)
	{
		DialogueManager->ShowMessage(SuccessMessage);
	}

	OnBoardingAllowed(PlayerCharacter);
}

bool ABoatInteractable::CanBoard(AProgressionManager* ProgressionManager) const
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

ACharacter* ABoatInteractable::GetPlayerCharacter() const
{
	return Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}