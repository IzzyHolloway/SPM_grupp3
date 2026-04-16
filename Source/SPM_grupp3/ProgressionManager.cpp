// Fill out your copyright notice in the Description page of Project Settings.


#include "ProgressionManager.h"

AProgressionManager::AProgressionManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AProgressionManager::AddFlag(FName FlagName)
{
	if (FlagName.IsNone())
	{
		return;
	}

	ProgressFlags.Add(FlagName);

	UE_LOG(LogTemp, Warning, TEXT("Added progression flag: %s"), *FlagName.ToString());
}

void AProgressionManager::RemoveFlag(FName FlagName)
{
	if (FlagName.IsNone())
	{
		return;
	}

	ProgressFlags.Remove(FlagName);

	UE_LOG(LogTemp, Warning, TEXT("Removed progression flag: %s"), *FlagName.ToString());
}

bool AProgressionManager::HasFlag(FName FlagName) const
{
	return ProgressFlags.Contains(FlagName);
}

void AProgressionManager::ClearAllFlags()
{
	ProgressFlags.Empty();

	UE_LOG(LogTemp, Warning, TEXT("Cleared all progression flags"));
}