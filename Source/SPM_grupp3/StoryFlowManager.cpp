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
		!ProgressionManager->HasFlag(Island1PuzzleSolvedFlag) &&
		!ProgressionManager->HasFlag(TalkedToListenerAfterPuzzleFlag) &&
		!ProgressionManager->HasFlag(SomeMelodyPiecesFoundFlag))
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

	// Final Island 1 state: the player has spoken to The Listener after solving the puzzle
	// and should now return to the boat.
	if (ProgressionManager->HasFlag(TalkedToListenerAfterPuzzleFlag))
	{
		ProgressionManager->SetCurrentObjectiveText(Island1SolvedObjectiveText);
		ProgressionManager->SetCurrentObjectiveID(Island1SolvedObjectiveID);
		return;
	}

	// The puzzle itself is solved, but the player still needs to return to The Listener
	// to receive the next story information.
	if (ProgressionManager->HasFlag(Island1PuzzleSolvedFlag))
	{
		ProgressionManager->SetCurrentObjectiveText(ReturnToListenerObjectiveText);
		ProgressionManager->SetCurrentObjectiveID(ReturnToListenerObjectiveID);
		return;
	}

	// All melody pieces found: return to The Listener.
	if (AreAllMelodyPiecesFound(ProgressionManager))
	{
		if (!ProgressionManager->HasFlag(AllMelodyPiecesFoundFlag))
		{
			ProgressionManager->AddFlag(AllMelodyPiecesFoundFlag);
		}

		if (!ProgressionManager->HasFlag(SomeMelodyPiecesFoundFlag))
		{
			ProgressionManager->AddFlag(SomeMelodyPiecesFoundFlag);
		}

		ProgressionManager->SetCurrentObjectiveText(ReturnToListenerObjectiveText);
		ProgressionManager->SetCurrentObjectiveID(ReturnToListenerObjectiveID);
		return;
	}

	// Some pieces found: still searching for melody pieces.
	if (HasAnyMelodyPiece(ProgressionManager))
	{
		if (!ProgressionManager->HasFlag(SomeMelodyPiecesFoundFlag))
		{
			ProgressionManager->AddFlag(SomeMelodyPiecesFoundFlag);
		}

		ProgressionManager->SetCurrentObjectiveText(StartPuzzle1Text);
		ProgressionManager->SetCurrentObjectiveID(StartPuzzle1ID);
		return;
	}

	// After Listener intro has been completed, but before any notes are found,
	// the player should begin searching for the melody pieces.
	if (ProgressionManager->HasFlag(TalkedToListenerIntroID))
	{
		ProgressionManager->SetCurrentObjectiveText(StartPuzzle1Text);
		ProgressionManager->SetCurrentObjectiveID(StartPuzzle1ID);
		return;
	}

	// Default starting state before the player speaks to The Listener.
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

bool AStoryFlowManager::HasAnyMelodyPiece(AProgressionManager* ProgressionManager) const
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

		if (ProgressionManager->HasFlag(Flag))
		{
			return true;
		}
	}

	return false;
}