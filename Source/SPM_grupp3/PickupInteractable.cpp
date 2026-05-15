// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupInteractable.h"

#include "DialogueManager.h"
#include "InventoryComponent.h"
#include "ProgressionManager.h"

#include "Engine/Engine.h"

#include "GameFramework/Character.h"
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

	// Add to inventory first.
	// If inventory is required and adding fails, do NOT collect the pickup.
	if (bAddToInventory)
	{
		const bool bAddedToInventory = TryAddToInventory();

		if (!bAddedToInventory && bRequireInventoryAddSuccess)
		{
			if (DialogueManager && !InventoryFailedMessage.IsEmpty())
			{
				DialogueManager->ShowMessage(InventoryFailedMessage);
			}

			return;
		}
	}

	// Only set progression flag after successful pickup.
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



bool APickupInteractable::TryAddToInventory() const
{
	if (ItemID.IsNone() || ItemID == "None" || ItemQuantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory add failed: Invalid ItemID or quantity."));
		return false;
	}

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory add failed: No player character found."));
		return false;
	}

	UInventoryComponent* InventoryComponent = PlayerCharacter->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory add failed: No InventoryComponent found on player."));
		return false;
	}

	const bool bSuccess = InventoryComponent->AddItemToInventory(ItemID, ItemQuantity);

	UE_LOG(LogTemp, Warning, TEXT("Tried to add item to inventory: %s x%d. Success: %s"),
		*ItemID.ToString(),
		ItemQuantity,
		bSuccess ? TEXT("true") : TEXT("false")
	);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.f,
			FColor::Green,
			FString::Printf(TEXT("Inventory add: %s x%d = %s"),
				*ItemID.ToString(),
				ItemQuantity,
				bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"))
		);
	}

	return bSuccess;
}