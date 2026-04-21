// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "PickupInteractable.generated.h"

/*
 * 
 * Pickups that adds progression flag when collected
 * 
 * Check also whther a set of required flags has been collected,
 * if so, unlock next progression step
 * 
 */

UCLASS()
class SPM_GRUPP3_API APickupInteractable : public AInteractableActor
{
	GENERATED_BODY()

public:
	virtual void Interact() override;

protected:
	//Message shown when this pickup is collected but the collection goal is not complete yet.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText PickupMessage = FText::FromString("I found something useful.");

	//Message shown when this pickup completes the current collection milestone.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText FinalPickupMessage = FText::FromString("Great, I found everything. Let's move on!");

	//Flag added when this specific pickup is collected.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName ProgressFlagToAdd;

	//All of these flags must exist before the completion flag is awarded.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	TArray<FName> RequiredFlagsForCompletion;

	//Flag added when all required flags for this collection milestone exist.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName CompletionFlagToAdd = "BoatReady";

	//Optional objective text to set when the collection milestone is completed.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FText ObjectiveTextOnCompletion = FText::FromString("Return to the boat.");

	//Optional objective ID to set when the collection milestone is completed.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FName ObjectiveIDOnCompletion = "ReturnToBoat";
};