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
		// No condition means this entry is always valid.
		return true;

	case EDialogueConditionType::RequiresFlag:
		// Entry is valid only if the required flag exists.
		return ProgressionManager && ProgressionManager->HasFlag(Entry.ConditionFlag);

	case EDialogueConditionType::BlockedByFlag:
		// Entry is valid only if the flag does NOT exist.
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

	// Go through dialogue entries in order and use the first one that matches.
	for (const FDialogueEntry& Entry : DialogueData->DialogueEntries)
	{
		if (!DoesEntryMatch(ProgressionManager, Entry))
		{
			continue;
		}

		if (Entry.DialogueLines.IsEmpty())
		{
			continue;
		}

		// If this entry should set a flag after the dialogue ends
		// Example. You talked with GuideNPC. After you talk with the GuideNPC, the flag 'TalkedWithGuide' should be active

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