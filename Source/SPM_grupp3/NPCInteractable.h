// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "DialogueEntry.h"
#include "NPCInteractable.generated.h"

class UDialogueDataAsset;
class AProgressionManager;
class UOutlineComponent;
class ADialogueManager;

/*
 * 
 * Interactable NPC that starts dialogue from DialogueDataAsset
 * 
 * Reads dialogue entries from data asset
 * Pick first dialogue entry whose condition matches progression state
 * Start Dialogue through DialogueManager
 * 
 * 
 */

UCLASS()
class SPM_GRUPP3_API ANPCInteractable : public AInteractableActor
{
	GENERATED_BODY()

public:
	virtual void Interact() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<UDialogueDataAsset> DialogueData;

	bool DoesEntryMatch(AProgressionManager* ProgressionManager, const FDialogueEntry& Entry) const;

private:
	// Outline suppressed while this NPC's dialogue is active, restored when it ends.
	UPROPERTY()
	TObjectPtr<UOutlineComponent> CachedOutline;

	// Manager we bound OnDialogueEnded to, kept so we can unbind in the handler.
	UPROPERTY()
	TObjectPtr<ADialogueManager> BoundDialogueManager;

	UFUNCTION()
	void HandleDialogueEnded();
};