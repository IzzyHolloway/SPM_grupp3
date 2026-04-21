// Fill out your copyright notice in the Description page of Project Settings.
#include "StoryFlowManager.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"

AStoryFlowManager::AStoryFlowManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStoryFlowManager::BeginPlay()
{
	Super::BeginPlay();

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		return;
	}

	// Set the starting objective only if the story has not already moved past it.
	// Like if the player is progressing in level2, level1 objectives are not gonna start
	if (!ProgressionManager->HasFlag(AllMelodyPiecesFoundFlag) &&
		!ProgressionManager->HasFlag(Island1SolvedFlag))
	{
		ProgressionManager->SetCurrentObjectiveText(InitialObjectiveText);
		ProgressionManager->SetCurrentObjectiveID(InitialObjectiveID);
	}
}

void AStoryFlowManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateStoryFlow();
}

void AStoryFlowManager::UpdateStoryFlow()
{
	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		return;
	}

	// Final Island 1 state: the player has completed the island and should return to the boat.
	if (ProgressionManager->HasFlag(Island1SolvedFlag))
	{
		ProgressionManager->SetCurrentObjectiveText(Island1SolvedObjectiveText);
		ProgressionManager->SetCurrentObjectiveID(Island1SolvedObjectiveID);
		return;
	}

	// Mid-state: the player has found all melody pieces and should return to The Listener.
	if (AreAllMelodyPiecesFound(ProgressionManager))
	{
		if (!ProgressionManager->HasFlag(AllMelodyPiecesFoundFlag))
		{
			ProgressionManager->AddFlag(AllMelodyPiecesFoundFlag);
		}

		ProgressionManager->SetCurrentObjectiveText(ReturnToListenerObjectiveText);
		ProgressionManager->SetCurrentObjectiveID(ReturnToListenerObjectiveID);
		return;
	}

	// Default state: the player is still searching for melody pieces.
	ProgressionManager->SetCurrentObjectiveText(InitialObjectiveText);
	ProgressionManager->SetCurrentObjectiveID(InitialObjectiveID);
}

bool AStoryFlowManager::AreAllMelodyPiecesFound(AProgressionManager* ProgressionManager) const
{
	if (!ProgressionManager)
	{
		return false;
	}

	for (const FName& Flag : MelodyPieceFlags)
	{
		if (Flag.IsNone())
		{
			continue;
		}

		if (!ProgressionManager->HasFlag(Flag))
		{
			return false;
		}
	}

	return true;
}