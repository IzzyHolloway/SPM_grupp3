// Fill out your copyright notice in the Description page of Project Settings.


#include "ProgressionManager.h"
#include "ObjectiveWidgetBase.h"

AProgressionManager::AProgressionManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AProgressionManager::BeginPlay()
{
	Super::BeginPlay();

	// Create the debug/prototype objective widget once at startup.
	if (ObjectiveWidgetClass)
	{
		ObjectiveWidgetInstance = CreateWidget<UObjectiveWidgetBase>(GetWorld(), ObjectiveWidgetClass);

		if (ObjectiveWidgetInstance)
		{
			ObjectiveWidgetInstance->AddToViewport();
			RefreshObjectiveWidget();
		}
	}
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

void AProgressionManager::SetCurrentObjectiveText(const FText& NewObjectiveText)
{
	CurrentObjectiveText = NewObjectiveText;

	UE_LOG(LogTemp, Warning, TEXT("Set current objective text: %s"), *CurrentObjectiveText.ToString());

	RefreshObjectiveWidget();
}

void AProgressionManager::SetCurrentObjectiveID(FName NewObjectiveID)
{
	CurrentObjectiveID = NewObjectiveID;

	UE_LOG(LogTemp, Warning, TEXT("Set current objective ID: %s"), *CurrentObjectiveID.ToString());

	RefreshObjectiveWidget();
}

FText AProgressionManager::GetCurrentObjectiveText() const
{
	return CurrentObjectiveText;
}

FName AProgressionManager::GetCurrentObjectiveID() const
{
	return CurrentObjectiveID;
}

void AProgressionManager::RefreshObjectiveWidget()
{
	if (!ObjectiveWidgetInstance)
	{
		return;
	}

	const FText ObjectiveIDText = FText::FromName(CurrentObjectiveID);
	ObjectiveWidgetInstance->SetObjectiveData(CurrentObjectiveText, ObjectiveIDText);
}