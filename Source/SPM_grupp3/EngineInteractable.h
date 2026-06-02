// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "EngineInteractable.generated.h"

class AProgressionManager;
class ADialogueManager;

UCLASS()
class SPM_GRUPP3_API AEngineInteractable : public AInteractableActor
{
	GENERATED_BODY()

public:
	virtual void Interact() override;

protected:
	// Optional prerequisite flag. Leave as None if the engine should always be interactable.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName RequiredProgressFlag = "TalkedToLighthouseEntity";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName EngineInstalledFlag = "LighthouseEngineInstalled";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName LightCutscenePlayedFlag = "LighthouseLightCutscenePlayed";

	// For testing only. If true, the cutscene flag is added immediately.
	// Later, set this to false and let the actual cutscene add the flag when finished.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	bool bAddLightCutsceneFlagImmediately = false;

	// Shown when the player tries to repair the engine before the story has reached that step.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	FText NotReadyMessage = FText::FromString("I should understand this place better before trying to repair the engine.");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	FText AlreadyInstalledMessage = FText::FromString("The engine is already installed.");

	// Called when the player tries to use the engine before the required story flag exists.
	UFUNCTION(BlueprintImplementableEvent, Category = "Engine")
	void OnEngineInteractionBlocked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Engine")
	void OnEngineInstalled();
};
