// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DialogueConditionType.generated.h"

UENUM(BlueprintType)
enum class EDialogueConditionType : uint8
{
	None UMETA(DisplayName = "None"),
	RequiresFlag UMETA(DisplayName = "Requires Flag"),
	BlockedByFlag UMETA(DisplayName = "Blocked By Flag")
};