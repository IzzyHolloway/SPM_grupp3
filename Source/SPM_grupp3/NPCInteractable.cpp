// Fill out your copyright notice in the Description page of Project Settings.

#include "NPCInteractable.h"
#include "DialogueManager.h"
#include "Kismet/GameplayStatics.h"

void ANPCInteractable::Interact()
{
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (DialogueManager)
	{
		DialogueManager->ShowMessage(NPCMessage);
	}
}