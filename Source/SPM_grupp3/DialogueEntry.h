// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DialogueLines.h"
#include "DialogueConditionType.h"
#include "DialogueEntry.generated.h"

USTRUCT(BlueprintType)
struct FDialogueEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	EDialogueConditionType ConditionType = EDialogueConditionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName ConditionFlag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueLines> DialogueLines;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	bool bSetFlagOnDialogueEnd = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName FlagToSetOnDialogueEnd;
};