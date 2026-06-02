// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "Components/AudioComponent.h"//Zoeys
#include "Sound/SoundBase.h"//Zoeys
#include "Sound/SoundAttenuation.h"//Zoeys
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

	/*
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine")
	FText AlreadyInstalledMessage = FText::FromString("The engine is already installed.");
	*/

	// Called when the player tries to use the engine before the required story flag exists.
	UFUNCTION(BlueprintImplementableEvent, Category = "Engine")
	void OnEngineInteractionBlocked();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Engine")
	void OnEngineInstalled();
	
	//Zoey strat
	
	// One-shot ljud som spelas när motorn installeras
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine|Sound")
	USoundBase* EngineInstalledOneShot = nullptr;

	// Loopande ljud  efter one-shot-ljudet är klart
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine|Sound")
	USoundBase* EngineLoopSound = nullptr;

	// Attenuation-inställningar för det loopande ljudet
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Engine|Sound")
	USoundAttenuation* EngineLoopAttenuation = nullptr;

private:
	// Referens till spawnad audio component för det loopande ljudet
	UPROPERTY()
	UAudioComponent* EngineLoopAudioComponent = nullptr;

	void OnOneShotFinished();
	//Zoey end

	
};
