// Fill out your copyright notice in the Description page of Project Settings.

#include "DoorInteractable.h"
#include "ProgressionManager.h"
#include "DialogueManager.h"
#include "Kismet/GameplayStatics.h"

ADoorInteractable::ADoorInteractable()
{
	
}

//Calling the Blueprint event when we interact with the door. 
void ADoorInteractable::Interact()
{
	if (bRequiresProgressionFlag)
	{
		if (RequiredProgressionFlag.IsNone())
		{
			UE_LOG(LogTemp, Warning, TEXT("DoorInteractable: Door requires progression flag, but RequiredProgressionFlag is None."));
			ShowLockedMessage();
			OnDoorLocked();
			return;
		}

		AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
		);

		if (!ProgressionManager)
		{
			UE_LOG(LogTemp, Warning, TEXT("DoorInteractable: No ProgressionManager found."));
			ShowLockedMessage();
			OnDoorLocked();
			return;
		}

		// Change HasFlag to whatever your ProgressionManager check function is called.
		if (!ProgressionManager->HasFlag(RequiredProgressionFlag))
		{
			ShowLockedMessage();
			OnDoorLocked();
			return;
		}
	}

	OnDoorInteract();
}

void ADoorInteractable::ShowLockedMessage()
{
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (!DialogueManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("DoorInteractable: No DialogueManager found."));
		return;
	}

	if (LockedMessage.IsEmpty())
	{
		DialogueManager->ShowMessage(FText::FromString(TEXT("The door is locked.")));
	}
	else
	{
		DialogueManager->ShowMessage(LockedMessage);
	}
}