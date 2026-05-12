// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorInteractable.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"

ADoorInteractable::ADoorInteractable()
{
	
}

//Calling the Blueprint event when we interact with the door. 
void ADoorInteractable::Interact()
{
	if (bRequiresProgressionFlag)
	{
		AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
		);

		if (!ProgressionManager)
		{
			UE_LOG(LogTemp, Warning, TEXT("DoorInteractable: No ProgressionManager found."));
			OnDoorLocked();
			return;
		}

		// Change HasFlag to whatever your ProgressionManager check function is called.
		if (!ProgressionManager->HasFlag(RequiredProgressionFlag))
		{
			OnDoorLocked();
			return;
		}
	}

	OnDoorInteract();
}
