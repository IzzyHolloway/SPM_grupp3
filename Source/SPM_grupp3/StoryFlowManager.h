// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoryFlowManager.generated.h"

class AProgressionManager;

/**
 * Central controller for story and objective flow.
 *
 * Responsibilities:
 * -Set the correct starting objective
 * -Watch progression flags and react when important milestones are reached
 * -Update current objective text and objective ID in ProgressionManager
 *
 * Version 1 only handles Island 1 flow, but the class is intended to grow
 * into a global story/progression controller for the whole game.
 */
UCLASS()
class SPM_GRUPP3_API AStoryFlowManager : public AActor
{
	GENERATED_BODY()

public:
	AStoryFlowManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Updates story flow based on the current progression state.
	void UpdateStoryFlow();

	
	
	/******************   LEVEL 1    ********************/
	
	//Returns true if all melody piece flags are currently collected.
	bool AreAllMelodyPiecesFound(AProgressionManager* ProgressionManager) const;
	
	// Returns true if any of the melody pieces are found
	bool HasAnyMelodyPiece(AProgressionManager* ProgressionManager) const;
	
	//bool HasTalkedToListenerIntro(AProgressionManager* ProgressionManager) const;
	
	// Island1 default objective
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FText InitialObjectiveText = FText::FromString("Explore Island 1");
	
	//Island1 default objective ID.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FName InitialObjectiveID = "ExploreIsland1";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FName TalkedToListenerIntroID = "TalkedToListenerIntro";
	
	//Starting objective for Island 1.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FText StartPuzzle1Text = FText::FromString("Find the missing melody pieces.");

	//Starting objective ID for Island 1.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FName StartPuzzle1ID = "FindMelodyPieces";
	
	

	/*
	//Starting objective for Island 1.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FText InitialObjectiveText = FText::FromString("Find the missing melody pieces.");

	//Starting objective ID for Island 1.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FName InitialObjectiveID = "FindMelodyPieces";
*/
	// Objective shown after all melody pieces are found.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FText ReturnToListenerObjectiveText = FText::FromString("Return to The Listener.");

	// Objective ID shown after all melody pieces are found.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FName ReturnToListenerObjectiveID = "ReturnToListener";

	//Objective shown after Island 1 is solved.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FText Island1SolvedObjectiveText = FText::FromString("Return to the boat.");

	//Objective ID shown after Island 1 is solved.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FName Island1SolvedObjectiveID = "ReturnToBoat";

	//Melody piece flags required to complete the collection phase.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	TArray<FName> MelodyPieceFlags =
	{
		"NotePiece1",
		"NotePiece2",
		"NotePiece3",
		"NotePiece4",
		"NotePiece5"
	};

	// Flag that marks that all required melody pieces have been found.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName AllMelodyPiecesFoundFlag = "AllMelodyPiecesFound";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName Island1PuzzleSolvedFlag = "Island1PuzzleSolved";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName TalkedToListenerAfterPuzzleFlag = "TalkedToListenerAfterPuzzle";
	
	// FLag that marks that some of the meody pieces have been found, but not all
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName SomeMelodyPiecesFoundFlag = "SomeMelodyPiecesFound";
	
	
	
	/***********************   NEXT LEVEL???    ****************************/
};
