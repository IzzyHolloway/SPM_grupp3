// Fill out your copyright notice in the Description page of Project Settings.

#include "EngineInteractable.h"

#include "DialogueManager.h"
#include "ProgressionManager.h"
#include "Components/AudioComponent.h" //Zoey
#include "Kismet/GameplayStatics.h"

void AEngineInteractable::Interact()
{
	Super::Interact();

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("EngineInteractable: No ProgressionManager found."));
		return;
	}

	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (!RequiredProgressFlag.IsNone() && !ProgressionManager->HasFlag(RequiredProgressFlag))
	{
		if (DialogueManager)
		{
			DialogueManager->ShowMessage(NotReadyMessage);
		}

		OnEngineInteractionBlocked();
		return;
	}

	if (ProgressionManager->HasFlag(EngineInstalledFlag))
	{
		if (DialogueManager)
		{
			DialogueManager->ShowMessage(AlreadyInstalledMessage);
		}

		return;
	}

	ProgressionManager->AddFlag(EngineInstalledFlag);

	if (bAddLightCutsceneFlagImmediately && !LightCutscenePlayedFlag.IsNone())
	{
		ProgressionManager->AddFlag(LightCutscenePlayedFlag);
	}
	
	//Zoey start
	// Spela one-shot-ljud och starta loop-ljud när one-shot är klart
	if (EngineInstalledOneShot)
	{
		UAudioComponent* OneShotComp = UGameplayStatics::SpawnSoundAtLocation(
			GetWorld(),
			EngineInstalledOneShot,
			GetActorLocation()
		);

		if (OneShotComp)
		{
			OneShotComp->OnAudioFinished.AddDynamic(this, &AEngineInteractable::OnOneShotFinished);
		}
		else
		{
			// Fallback om spawn misslyckades — starta loop direkt
			OnOneShotFinished();
		}
	}
	else
	{
		// Inget one-shot konfigurerat — starta loop direkt
		OnOneShotFinished();
	}

	
// Zoey end

	OnEngineInstalled();
	
	
}
//Zoey start
	void AEngineInteractable::OnOneShotFinished()
	{
		if (!EngineLoopSound)
		{
			return;
		}

		EngineLoopAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
			GetWorld(),
			EngineLoopSound,
			GetActorLocation(),
			FRotator::ZeroRotator,
			1.0f,   // VolumeMultiplier
			1.0f,   // PitchMultiplier
			0.0f,   // StartTime
			EngineLoopAttenuation
		);

		if (EngineLoopAudioComponent)
		{
			EngineLoopAudioComponent->bAutoDestroy = false;
		}
	}
	//Zoey end