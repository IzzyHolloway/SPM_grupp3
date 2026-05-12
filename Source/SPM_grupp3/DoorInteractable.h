// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "DoorInteractable.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GRUPP3_API ADoorInteractable : public AInteractableActor
{
	GENERATED_BODY()
	
public:
	ADoorInteractable();
	
	//Function created to be implemented in blueprints. 
	UFUNCTION(BlueprintImplementableEvent)
	void OnDoorInteract();
	
	virtual void Interact() override;
	
protected:
	// If true, this door requires a progression flag before it can be used.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Progression")
	bool bRequiresProgressionFlag = false;

	// Example: Island3HouseAccessAllowed
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Progression", meta = (EditCondition = "bRequiresProgressionFlag"))
	FName RequiredProgressionFlag;

	// Optional event for locked feedback in Blueprint.
	UFUNCTION(BlueprintImplementableEvent)
	void OnDoorLocked();
	
};
