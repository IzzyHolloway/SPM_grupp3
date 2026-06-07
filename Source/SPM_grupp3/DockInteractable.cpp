// Fill out your copyright notice in the Description page of Project Settings.

#include "DockInteractable.h"
#include "DialogueManager.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"

void ADockInteractable::Interact()
{
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		return;
	}

	if (!CanUseDock(ProgressionManager))
	{
		if (DialogueManager && !BlockedMessage.IsEmpty())
		{
			DialogueManager->ShowMessage(BlockedMessage);
		}

		return;
	}

	if (DialogueManager && !SuccessMessage.IsEmpty())
	{
		DialogueManager->ShowMessage(SuccessMessage);
	}

	if (!ProgressFlagToAddOnSuccess.IsNone())
	{
		ProgressionManager->AddFlag(ProgressFlagToAddOnSuccess);
	}

	OnDockSuccess();
}

bool ADockInteractable::CanUseDock(AProgressionManager* ProgressionManager) const
{
	if (!ProgressionManager)
	{
		return false;
	}

	if (RequiredProgressFlag.IsNone())
	{
		return true;
	}

	return ProgressionManager->HasFlag(RequiredProgressFlag);
}