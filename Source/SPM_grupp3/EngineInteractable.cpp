// Fill out your copyright notice in the Description page of Project Settings.

#include "EngineInteractable.h"

#include "DialogueManager.h"
#include "ProgressionManager.h"
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

	OnEngineInstalled();
}
