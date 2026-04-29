// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
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
	// Message shown when the pickup is collected. (debug, just to see if it picked up)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText PickupMessage = FText::FromString("I found something useful.");
	
	// Message shown if the player tries to pick this before requirements are met
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FText BlockedPickupMessage = FText::FromString("Hmm... I wanna pick it up, but I have nowhere to put it.");

	// Progression flag added when this pickup is collected
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName ProgressFlagToAdd;
	
	// Progression flag added when this pickup is collected.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	TArray<FName> RequiredFlagsToPickup;
	
	/****** HELPER ******/
	bool CanPickup(class AProgressionManager* ProgressionManager) const;
};