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
};