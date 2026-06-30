// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DialogueLines.h"

#include "DialogueEntry.generated.h"

USTRUCT(BlueprintType)
struct FDialogueEntry
{
	GENERATED_BODY()

	// DialogueID for checking which dialogue you are editing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName DialogueID;
	
	//All of these flags must exist for this dialogue entry to match.
	//Leave empty if the entry should not require any progression flags.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conditions")
	TArray<FName> RequiredFlags;
	
	// None of these flags may exist for this dialogue entry to match.
	//Leave empty if the entry should not block on progression flags.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conditions")
	TArray<FName> BlockedFlags;

	//Dialogue lines played if this entry matches.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueLines> DialogueLines;

	/**
	 * Raw import text used as a quick authoring helper.
	 *
	 * Expected format:
	 * SpeakerName|Line text
	 * SpeakerName|Line text
	 *
	 * Example:
	 * The Listener|You found some...
	 * The Listener|I can almost hear it...
	 * The Little Creature|I'll keep looking.
	 *
	 * This field is only for easier content entry.
	 * It is not used directly by the runtime dialogue system.
	 */
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Import", meta = (MultiLine = "true"))
	FText RawDialogueImportText;

	//If true, add a progression flag when this dialogue ends.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	bool bSetFlagOnDialogueEnd = false;

	// Progression flag added when the dialogue ends.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName FlagToSetOnDialogueEnd;

	// If true, this dialogue advances itself (no player input). Use for cutscene dialogue like the
	// lighthouse: each line shows for AutoAdvanceSecondsPerLine, then it moves on / ends on its own.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutscene")
	bool bAutoAdvance = false;

	// Seconds each line stays on screen when bAutoAdvance is true. For a single-line cutscene this is
	// effectively "show the line for this long, then end" (which fires OnDialogueEnded).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutscene", meta = (EditCondition = "bAutoAdvance", ClampMin = "0.1"))
	float AutoAdvanceSecondsPerLine = 3.0f;
};