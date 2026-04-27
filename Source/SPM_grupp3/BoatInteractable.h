// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "BoatInteractable.generated.h"

/**
 * Boat interaction that checks progression flags to decide
 * whether the player is allowed to continue.
 * 
 * Reads progression state from ProgressionManager
 */

UCLASS()
class SPM_GRUPP3_API ABoatInteractable : public AInteractableActor
{
	GENERATED_BODY()

public:
	virtual void Interact() override;

protected:
	// Flag required before the boat can be used.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName RequiredProgressFlag = "BoatReady";

	// Message shown when player has not completed required progression yet.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText BlockedMessage = FText::FromString("Hmm... I feel like something is missing...");

	// Message shown when player is allowed to continue.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText SuccessMessage = FText::FromString("Let's move on!");
	
	/**** HELPER ****/
	bool CanUseBoat(class AProgressionManager* ProgressionManager) const;
};
