// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "Sound/SoundBase.h"
#include "PickupInteractable.generated.h"

/**
 * Simple interactable pickup.
 *
 * Responsibilities:
 * - Add a progression flag when collected
 * - Show a short gameplay message
 * - Destroy itself after collection
 *
 * This class should stay simple.
 * Higher-level story progression is handled by StoryFlowManager.
 */
UCLASS()
class SPM_GRUPP3_API APickupInteractable : public AInteractableActor
{
	GENERATED_BODY()

public:
	virtual void Interact() override;

protected:
	/******** CONNECT TO INVENTORY ********/
	
	// If true, this item should be aded to the player's InventoryComponent
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Inventory")
	bool bAddToInventory = true;
	
	// Item ID used by InventoryComponent
	// It must match EXACTLY
	// Name_None is Unreal's way of saying nothing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Inventory")
	FName ItemID = NAME_None;
	
	// How many
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Inventory")
	int32 ItemQuantity = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Inventory")
	bool bRequireInventoryAddSuccess = true;
	
	
	
	/******** DIALOGUE / MESSAGES ********/
	// Message shown when the pickup is collected. (debug, just to see if it picked up)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText PickupMessage = FText::FromString("I found something useful.");
	
	// Message shown if the player tries to pick this before requirements are met
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FText BlockedPickupMessage = FText::FromString("I can't pick it up");
	
	// Message shown if this pickup should go into inventory, but inventory is missing or full.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Inventory")
	FText InventoryFailedMessage = FText::FromString("I can't carry this right now.");

	
	/******** PROGRESSION / FLAGS ********/
	
	// Progression flag added when this pickup is collected
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName ProgressFlagToAdd;
	
	// Progression flag added when this pickup is collected.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	TArray<FName> RequiredFlagsToPickup;
	
	/****** HELPERs ******/
	bool CanPickup(class AProgressionManager* ProgressionManager) const;
	
	bool TryAddToInventory() const;
	
	/****** SOUND *******/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Audio")
	USoundBase* PickupSound = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Audio")
	float PickupSoundVolume = 1.0f;
};