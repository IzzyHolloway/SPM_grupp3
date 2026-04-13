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
};
