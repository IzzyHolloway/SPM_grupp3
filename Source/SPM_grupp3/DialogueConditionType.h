// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DialogueConditionType.generated.h"

/*
 * Condition types used by dialogue entries
 * 
 * None					-> Entry is always valid
 * Requires Flag		-> Entry is valid only if the specifies flag exists.
 * Blocked by Flag		->Entry is valid only if the specified flag does NOT exist
 * 
 * INFO: If you have a better naming to these to make it more understandlable, great.
 * 
 */

UENUM(BlueprintType)
enum class EDialogueConditionType : uint8
{
	None UMETA(DisplayName = "None"),
	RequiresFlag UMETA(DisplayName = "Requires Flag"),
	BlockedByFlag UMETA(DisplayName = "Blocked By Flag")
};