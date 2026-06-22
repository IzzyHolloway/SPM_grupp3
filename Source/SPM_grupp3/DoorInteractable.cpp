// Fill out your copyright notice in the Description page of Project Settings.

#include "DoorInteractable.h"
#include "ProgressionManager.h"
#include "DialogueManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"

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

	// Arm the safety net: if the Blueprint transition stalls, this frees the player anyway.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DoorFailsafeTimer, this,
			&ADoorInteractable::RestorePlayerControlIfStuck, DoorFailsafeSeconds, false);
	}
}

void ADoorInteractable::RestorePlayerControlIfStuck()
{
	// The Blueprint keeps its own "bIsGoingThruDoor" flag true for the duration of the transition
	// and sets it false at the very end. If it's still true now, the latent fade/delay chain
	// never finished and the player is likely frozen -- otherwise everything went fine and we do
	// nothing (so we never interfere with normal play, menus or dialogue).
	const FBoolProperty* GoingProp = CastField<FBoolProperty>(GetClass()->FindPropertyByName(TEXT("bIsGoingThruDoor")));
	if (!GoingProp || !GoingProp->GetPropertyValue_InContainer(this))
	{
		// Either we can't read the flag, or the transition finished normally -- do nothing.
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("DoorInteractable: door transition stalled -- restoring player control."));

	// Undo the two ways the character's movement gets locked in this project.
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->ResetIgnoreMoveInput();
		PC->ResetIgnoreLookInput();
	}

	if (ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		if (UCharacterMovementComponent* Movement = PlayerCharacter->GetCharacterMovement())
		{
			if (Movement->MovementMode == MOVE_None)
			{
				Movement->SetMovementMode(MOVE_Walking);
			}
		}
	}

	// Clear the stuck flag so the door is usable again.
	if (GoingProp)
	{
		GoingProp->SetPropertyValue_InContainer(this, false);
	}
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