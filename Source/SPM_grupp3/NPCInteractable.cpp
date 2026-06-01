// Fill out your copyright notice in the Description page of Project Settings.
#include "NPCInteractable.h"
#include "DialogueManager.h"
#include "ProgressionManager.h"
#include "DialogueDataAsset.h"
#include "OutlineComponent.h"
#include "Kismet/GameplayStatics.h"

bool ANPCInteractable::DoesEntryMatch(AProgressionManager* ProgressionManager, const FDialogueEntry& Entry) const
{
	// If progression checks are needed but the manager does not exist, fail safely.
	if (!ProgressionManager && (!Entry.RequiredFlags.IsEmpty() || !Entry.BlockedFlags.IsEmpty()))
	{
		return false;
	}

	// Every required flag must exist.
	for (const FName& RequiredFlag : Entry.RequiredFlags)
	{
		if (RequiredFlag.IsNone())
		{
			continue;
		}

		if (!ProgressionManager->HasFlag(RequiredFlag))
		{
			return false;
		}
	}

	// None of the blocked flags may exist.
	for (const FName& BlockedFlag : Entry.BlockedFlags)
	{
		if (BlockedFlag.IsNone())
		{
			continue;
		}

		if (ProgressionManager->HasFlag(BlockedFlag))
		{
			return false;
		}
	}

	return true;
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

		// Hide this NPC's proximity outline while we're talking, and restore it when the
		// dialogue ends. SetOutlineSuppressed re-checks the real overlap state on release,
		// so the outline only comes back if the player is still standing next to the NPC.
		CachedOutline = FindComponentByClass<UOutlineComponent>();
		if (CachedOutline)
		{
			CachedOutline->SetOutlineSuppressed(true);
			BoundDialogueManager = DialogueManager;
			DialogueManager->OnDialogueEnded.AddUniqueDynamic(this, &ANPCInteractable::HandleDialogueEnded);
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

void ANPCInteractable::HandleDialogueEnded()
{
	if (CachedOutline)
	{
		CachedOutline->SetOutlineSuppressed(false);
		CachedOutline = nullptr;
	}

	if (BoundDialogueManager)
	{
		BoundDialogueManager->OnDialogueEnded.RemoveDynamic(this, &ANPCInteractable::HandleDialogueEnded);
		BoundDialogueManager = nullptr;
	}
}