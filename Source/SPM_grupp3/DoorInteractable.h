// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "Engine/TimerHandle.h"
#include "DoorInteractable.generated.h"

class ADialogueManager;

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
	
	// Message shown if player tries to use the door too early.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText LockedMessage = FText::FromString(TEXT("The door is locked."));
	
	void ShowLockedMessage();

private:
	// Safety net for the "stuck in a door / can't move" bug. The door transition lives in the
	// Blueprint and frees the player only at the end of a latent fade+delay chain; if any latent
	// step never reports finished, the player is left unable to move. A few seconds after the
	// door is used we force control back -- but ONLY if the transition never finished, so normal
	// play (and menus/dialogue entered afterwards) is untouched.
	FTimerHandle DoorFailsafeTimer;

	// How long after using a door to check for a stalled transition. Keep comfortably longer
	// than the normal fade-in + delay + fade-out.
	UPROPERTY(EditAnywhere, Category = "Door")
	float DoorFailsafeSeconds = 6.f;

	void RestorePlayerControlIfStuck();

};
