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
	
	
};
