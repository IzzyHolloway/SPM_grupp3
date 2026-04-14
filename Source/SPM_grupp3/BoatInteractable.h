// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "BoatInteractable.generated.h"

UCLASS()
class SPM_GRUPP3_API ABoatInteractable : public AInteractableActor
{
	GENERATED_BODY()

public:
	virtual void Interact() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText BlockedMessage = FText::FromString("Hmm.. I feel like something is missing...");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText SuccessMessage = FText::FromString("Let's move on!");
};