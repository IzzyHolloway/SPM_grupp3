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
	// Message shown when pickup is collected, but progression is not complete yet.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText PickupMessage = FText::FromString("I found something useful.");

	// Message when this pickup completes the current collection goal
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText FinalPickupMessage = FText::FromString("Great, I found everything. Let's move on!");
	
	// Flag added when this specific pickup is collected
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName ProgressFlagToAdd;
	
	// Flags that must exist before the next step is unlocked
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	TArray<FName> RequiredFlagsForCompletion;
	
	// Flag added when all required pickup have been collected
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	TArray<FName> RequiredFlagsForPickup;
	
	// Flag added when all required pickup flags have been collected
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName CompletionFlagToAdd = "BoatReady";
	
};