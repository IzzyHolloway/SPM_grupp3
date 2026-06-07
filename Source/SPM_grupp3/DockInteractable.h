// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "DockInteractable.generated.h"

class AProgressionManager;

/**
 * Interactable used for docking at islands / piers.
 *
 * Responsibilities:
 * - Check whether the player is allowed to dock here
 * - Show blocked/success messages
 * - Add progression flags when docking succeeds
 * - Expose a Blueprint event for camera changes, teleporting, animations, etc.
 *
 * Boat movement can stay free.
 * Docking/progression is handled here.
 */

UCLASS()
class SPM_GRUPP3_API ADockInteractable : public AInteractableActor
{
	GENERATED_BODY()

public:
	virtual void Interact() override;

protected:
	//Required flag before this dock can be used. Leave empty if always available.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName RequiredProgressFlag;

	// Optional flag added when docking succeeds.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName ProgressFlagToAddOnSuccess;

	// Message shown if the player is not allowed to dock yet.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText BlockedMessage = FText::FromString("I should finish what I came here to do first.");

	// Message shown when docking succeeds.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText SuccessMessage = FText::FromString("Let's dock here.");

	// Returns true if this dock can currently be used.
	bool CanUseDock(AProgressionManager* ProgressionManager) const;

	/**
	 * Blueprint hook called when docking succeeds.
	 *
	 * Use this in Blueprint for:
	 * - changing camera
	 * - teleporting player
	 * - playing boat arrival animation
	 * - enabling island controls
	 */
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Dock")
	void OnDockSuccess();
};