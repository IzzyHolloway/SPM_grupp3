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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName EngineInstalledFlag = "LighthouseEngineInstalled";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName LightCutscenePlayedFlag = "LighthouseLightCutscenePlayed";

	// For testing only. If true, the cutscene flag is added immediately.
	// Later, set this to false and let the actual cutscene add the flag when finished.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	bool bAddLightCutsceneFlagImmediately = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	FText AlreadyInstalledMessage = FText::FromString("The engine is already installed.");

	UFUNCTION(BlueprintImplementableEvent, Category = "Engine")
	void OnEngineInstalled();
};