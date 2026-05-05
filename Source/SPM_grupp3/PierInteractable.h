// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "PierInteractable.generated.h"

class AProgressionManager;
class ADialogueManager;
class ACharacter;
class USceneComponent;

UCLASS()
class SPM_GRUPP3_API APierInteractable : public AInteractableActor
{
	GENERATED_BODY()

public:
	APierInteractable();

	virtual void Interact() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pier")
	TObjectPtr<USceneComponent> PlayerExitPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName RequiredProgressFlag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName ProgressFlagToAddOnSuccess = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText BlockedMessage = FText::FromString("I can't dock here yet.");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText SuccessMessage = FText::FromString("Let's get off here.");

	// Implemented in Blueprint.
	// This should call Paula's BP_PaulaBoat -> ExitBoatAtPier.
	UFUNCTION(BlueprintImplementableEvent, Category = "Boat")
	void OnDockingAllowed(ACharacter* PlayerCharacter, FVector ExitLocation);

private:
	bool CanDock(AProgressionManager* ProgressionManager) const;
	void AddSuccessFlag(AProgressionManager* ProgressionManager) const;
	ACharacter* GetPlayerCharacter() const;
};