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

	// Decide where the story should begin based on existing progression flags.
	// For now, if ArrivedIsland1 exists, we start in Island 1 flow.
	if (ProgressionManager->HasFlag(ArrivedIsland1Flag))
	{
		SetStoryState(EStoryState::Island1_Explore);
	}
	else
	{
		SetStoryState(EStoryState::Home_Explore);
	}

	UpdateStoryFlow();
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

	// If the player has reached Island 1, use Island 1 flow.
	if (ProgressionManager->HasFlag(ArrivedIsland1Flag))
	{
		UpdateIsland1Flow(ProgressionManager);
		return;
	}

	// Otherwise, use the home intro flow.
	UpdateHomeFlow(ProgressionManager);
}

void AStoryFlowManager::UpdateHomeFlow(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager)
	{
		return;
	}

	const bool bHasLantern = ProgressionManager->HasFlag(PickedUpLanternFlag);
	const bool bHasMatches = ProgressionManager->HasFlag(PickedUpMatchesFlag);
	const bool bHasLitLantern = ProgressionManager->HasFlag(LitLanternFlag);

	// Final home state: the player has the lit lantern and can leave.
	if (bHasLitLantern)
	{
		if (!ProgressionManager->HasFlag(HomeReadyForBoatFlag))
		{
			ProgressionManager->AddFlag(HomeReadyForBoatFlag);
		}

		SetStoryState(EStoryState::Home_ReadyForBoat);
		SetObjective(ProgressionManager, HomeReadyForBoatObjectiveText, HomeReadyForBoatObjectiveID);
		return;
	}

	// The player has both crafting ingredients but has not crafted the lit lantern yet.
	if (bHasLantern && bHasMatches)
	{
		SetStoryState(EStoryState::Home_CraftLantern);
		SetObjective(ProgressionManager, HomeCraftLanternObjectiveText, HomeCraftLanternObjectiveID);
		return;
	}

	// Default intro state.
	SetStoryState(EStoryState::Home_Explore);
	SetObjective(ProgressionManager, HomeExploreObjectiveText, HomeExploreObjectiveID);
}

void AStoryFlowManager::UpdateIsland1Flow(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager)
	{
		return;
	}

	// Final Island 1 state:
	// The player has talked to The Listener after solving the puzzle.
	if (ProgressionManager->HasFlag(TalkedToListenerAfterPuzzleFlag))
	{
		SetStoryState(EStoryState::Island1_ReadyToLeave);
		SetObjective(ProgressionManager, ReturnToBoatObjectiveText, ReturnToBoatObjectiveID);
		return;
	}

	// Puzzle solved, but the player still needs to return to The Listener.
	if (ProgressionManager->HasFlag(Island1PuzzleSolvedFlag))
	{
		SetStoryState(EStoryState::Island1_PuzzleSolved);
		SetObjective(ProgressionManager, ReturnToListenerObjectiveText, ReturnToListenerObjectiveID);
		return;
	}

	// All melody pieces found.
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

		SetStoryState(EStoryState::Island1_ReturnToListener);
		SetObjective(ProgressionManager, ReturnToListenerObjectiveText, ReturnToListenerObjectiveID);
		return;
	}

	// Some melody pieces found.
	if (HasAnyMelodyPiece(ProgressionManager))
	{
		if (!ProgressionManager->HasFlag(SomeMelodyPiecesFoundFlag))
		{
			ProgressionManager->AddFlag(SomeMelodyPiecesFoundFlag);
		}

		SetStoryState(EStoryState::Island1_CollectMelodyPieces);
		SetObjective(ProgressionManager, FindMelodyPiecesObjectiveText, FindMelodyPiecesObjectiveID);
		return;
	}

	// The Listener intro is done, but no pieces have been found yet.
	if (ProgressionManager->HasFlag(TalkedToListenerIntroFlag))
	{
		SetStoryState(EStoryState::Island1_CollectMelodyPieces);
		SetObjective(ProgressionManager, FindMelodyPiecesObjectiveText, FindMelodyPiecesObjectiveID);
		return;
	}

	// Default Island 1 state before talking to The Listener.
	SetStoryState(EStoryState::Island1_Explore);
	SetObjective(ProgressionManager, Island1ExploreObjectiveText, Island1ExploreObjectiveID);
}

void AStoryFlowManager::SetStoryState(EStoryState NewState)
{
	if (CurrentStoryState == NewState)
	{
		return;
	}

	CurrentStoryState = NewState;

	UE_LOG(LogTemp, Warning, TEXT("Story state changed to: %s"), *UEnum::GetValueAsString(CurrentStoryState));
}

void AStoryFlowManager::SetObjective(AProgressionManager* ProgressionManager, const FText& NewText, FName NewID) const
{
	if (!ProgressionManager)
	{
		return;
	}

	// Avoid repeatedly setting the same objective every tick.
	if (ProgressionManager->GetCurrentObjectiveID() == NewID)
	{
		return;
	}

	ProgressionManager->SetCurrentObjectiveText(NewText);
	ProgressionManager->SetCurrentObjectiveID(NewID);
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