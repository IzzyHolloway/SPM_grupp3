// Fill out your copyright notice in the Description page of Project Settings.

#include "NPCInteractable.h"
#include "DialogueManager.h"
#include "CharacterAimi.h"
#include "Kismet/GameplayStatics.h"

void ANPCInteractable::Interact()
{
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (!DialogueManager)
	{
		return;
	}

	ACharacterAimi* PlayerCharacter = Cast<ACharacterAimi>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
	);

	if (!PlayerCharacter)
	{
		return;
	}

	const TArray<FDialogueLines>* SelectedDialogue = nullptr;

	if (PlayerCharacter->HasRequiredItems())
	{
		SelectedDialogue = &DialogueAfterRequirement;
	}
	else
	{
		SelectedDialogue = &DialogueBeforeRequirement;
	}

	if (!SelectedDialogue || SelectedDialogue->IsEmpty())
	{
		return;
	}

	DialogueManager->StartDialogue(*SelectedDialogue);
}