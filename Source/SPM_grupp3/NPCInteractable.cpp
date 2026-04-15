// Fill out your copyright notice in the Description page of Project Settings.
#include "NPCInteractable.h"
#include "DialogueManager.h"
#include "ProgressionManager.h"
#include "DialogueDataAsset.h"
#include "Kismet/GameplayStatics.h"

bool ANPCInteractable::DoesEntryMatch(AProgressionManager* ProgressionManager, const FDialogueEntry& Entry) const
{
	switch (Entry.ConditionType)
	{
	case EDialogueConditionType::None:
		return true;

	case EDialogueConditionType::RequiresFlag:
		return ProgressionManager && ProgressionManager->HasFlag(Entry.ConditionFlag);

	case EDialogueConditionType::BlockedByFlag:
		return !ProgressionManager || !ProgressionManager->HasFlag(Entry.ConditionFlag);

	default:
		return false;
	}
}

void ANPCInteractable::Interact()
{
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (!DialogueManager || !DialogueData)
	{
		return;
	}

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	for (const FDialogueEntry& Entry : DialogueData->DialogueEntries)
	{
		if (DoesEntryMatch(ProgressionManager, Entry))
		{
			if (!Entry.DialogueLines.IsEmpty())
			{
				if (Entry.bSetFlagOnDialogueEnd && !Entry.FlagToSetOnDialogueEnd.IsNone())
				{
					DialogueManager->StartDialogueWithFlag(Entry.DialogueLines, Entry.FlagToSetOnDialogueEnd);
				}
				else
				{
					DialogueManager->StartDialogue(Entry.DialogueLines);
				}

				return;
			}
		}
	}
}