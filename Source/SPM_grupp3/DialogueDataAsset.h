// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogueEntry.h"
#include "DialogueDataAsset.generated.h"

/*
 * Data asset containing dialogue entries for one NPC or one reusable conversation set
 * 
 * Keeping dialogue in a data asset makes it easier for teammates to edit content without touching the NPC class itself
 * 
 */

UCLASS(BlueprintType)
class SPM_GRUPP3_API UDialogueDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueEntry> DialogueEntries;
	
	// Button that Rebuilds DialogueLines for every entry that has RawDialogueImportText filled in
	// Useful if you only changed one entry and do not want to rebuild everything
	UFUNCTION(CallInEditor, Category = "Import")
	void RebuildAllDialogueLinesFromRawText();
	
	// Rebuilds Dialoguelines for a single entry by index
	// Useful if you only changed one entry and do not want to rebuild everything
	UFUNCTION(CallInEditor, Category = "Import")
	void RebuildDialogueLinesFromRawTextByIndex(int32 EntryIndex);
	
private:
	/*
	 * Internal helper that parses one raw text block into dialogue lines
	 * 
	 * Expected format per line:
	 * SpeakerName|Line text
	 * Need to separate the lines with new line (enter button)
	 * 
	 */
	void ParseRawDialogueText(const FString& RawText, TArray<FDialogueLines>& Outlines) const;
};