// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "PickupInteractable.generated.h"

UCLASS()
class SPM_GRUPP3_API APickupInteractable : public AInteractableActor
{
	GENERATED_BODY()

public:
	virtual void Interact() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText PickupMessage = FText::FromString("Found an item!");
};