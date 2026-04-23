// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueDataAsset.h"

void UDialogueDataAsset::RebuildAllDialogueLinesFromRawText()
{
	for (int32 EntryIndex = 0; EntryIndex < DialogueEntries.Num(); ++EntryIndex)
	{
		FDialogueEntry& Entry = DialogueEntries[EntryIndex];

		// Skip entries that do not currently have any raw import text.
		if (Entry.RawDialogueImportText.ToString().TrimStartAndEnd().IsEmpty())
		{
			continue;
		}

		ParseRawDialogueText(Entry.RawDialogueImportText.ToString(), Entry.DialogueLines);
	}

	// Mark this asset as changed in the editor so Unreal knows it should be saved.
	MarkPackageDirty();
}

void UDialogueDataAsset::RebuildDialogueLinesFromRawTextByIndex(int32 EntryIndex)
{
	if (!DialogueEntries.IsValidIndex(EntryIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("RebuildDialogueLinesFromRawTextByIndex: Invalid entry index %d"), EntryIndex);
		return;
	}

	FDialogueEntry& Entry = DialogueEntries[EntryIndex];
	ParseRawDialogueText(Entry.RawDialogueImportText.ToString(), Entry.DialogueLines);

	// Mark this asset as changed in the editor so Unreal knows it should be saved.
	MarkPackageDirty();
}

void UDialogueDataAsset::ParseRawDialogueText(const FString& RawText, TArray<FDialogueLines>& OutLines) const
{
	OutLines.Empty();

	TArray<FString> RawLines;
	RawText.ParseIntoArrayLines(RawLines, true);

	for (const FString& RawLine : RawLines)
	{
		const FString TrimmedLine = RawLine.TrimStartAndEnd();

		// Ignore empty lines completely.
		if (TrimmedLine.IsEmpty())
		{
			continue;
		}

		FString SpeakerPart;
		FString TextPart;

		// Split on the first "|" character.
		// Left side becomes the speaker name, right side becomes the actual dialogue text.
		if (!TrimmedLine.Split(TEXT("|"), &SpeakerPart, &TextPart))
		{
			UE_LOG(LogTemp, Warning, TEXT("Dialogue import skipped malformed line: %s"), *TrimmedLine);
			continue;
		}

		SpeakerPart = SpeakerPart.TrimStartAndEnd();
		TextPart = TextPart.TrimStartAndEnd();

		// Skip lines that do not contain useful text.
		if (TextPart.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Dialogue import skipped line with empty text: %s"), *TrimmedLine);
			continue;
		}

		FDialogueLines NewLine;
		NewLine.SpeakerName = FText::FromString(SpeakerPart);
		NewLine.LineText = FText::FromString(TextPart);

		OutLines.Add(NewLine);
	}
}