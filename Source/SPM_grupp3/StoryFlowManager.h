// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoryFlowManager.generated.h"

class AProgressionManager;

/*
 * Story States
 * 
 * Big event "where are we int the story?"
 * Smaller details are handled with progression flags
 * 
 */

UENUM(BlueprintType)
enum class EStoryState : uint8
{
	// INTRO / Home
	// Format: Place - What to do
	Home_Explore UMETA(DisplayName = "Home - Explore"),
	Home_FindLight UMETA(DisplayName = "Home - Find Light"),
	Home_CraftLantern UMETA(DisplayName = "Home - Craft Lantern"),
	Home_ReadyForBoat UMETA(DisplayName = "Home - Ready for Boat"),
	
	// Island 1
	Island1_Explore UMETA(DisplayName = "Island1 - Explore"),
	Island1_CollectMelodyPieces UMETA(DisplayName = "Island1 - Collect Melody Pieces"),
	Island1_ReturnToListener UMETA(DisplayName = "Island1 - Return to Listener"),
	Island1_PuzzleSolved UMETA(DisplayName = "Island1 - Puzzle Solved"),
	Island1_ReadyToLeave UMETA(DisplayName = "Island1 - Ready to Leave"),
	
	// Island 2
	Island2_Explore UMETA(DisplayName = "Island2 - Explore"),
	Island2_FindGramophoneParts UMETA(DisplayName = "Island2 - Find Gramophone Parts"),
	Island2_CraftGramophoneMechanism UMETA(DisplayName = "Island2 - Craft Gramophone Mechanism"),
	Island2_RepairGramophone UMETA(DisplayName = "Island2 - Repair Gramophone"),
	Island2_PlayGramophone UMETA(DisplayName = "Island2 - Play Gramophone"),
	Island2_TalkToNPC UMETA(DisplayName = "Island2 - Talk to NPC"),
	Island2_ReadyToLeave UMETA(DisplayName = "Island2 - Ready to Leave"),
};

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
 * 
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
	
	// Changes current story state and logs tha change
	void SetStoryState(EStoryState NewState);
	
	// Updates current objective only when needed
	void SetObjective(AProgressionManager* ProgressionManager, const FText& NewText, FName NewID) const;
	
	// The Currebt Story State
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	EStoryState CurrentStoryState = EStoryState::Home_Explore;
	
	/******************   INTRO (HOUSE)    ********************/
	// The Little creature needs to read the diary
	// The Little creature needs to pickup a torch/lamp and lighter/matches?
	// The little creature needs to craft a lit lamp and then it can progress to explore.
	
	// Handles the intro/home flow
	void UpdateHomeFlow(AProgressionManager* ProgressionManager);
	
	/** Intro/Home Flags **/
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Flags")
	//FName PickedUpBackpackFlag = "PickedUpBackpack";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Flags")
	FName PickedUpLanternFlag = "PickedUpLantern";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Flags")
	FName PickedUpMatchesFlag = "PickUpMatches";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Flags")
	FName LitLanternFlag = "LitLantern";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Flags")
	FName HomeReadyForBoatFlag = "HomeReadyForBoat";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Flags")
	FName ArrivedIsland1Flag = "ArrivedIsland1";
	
	/** Intro/Home Objectives **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FText HomeExploreObjectiveText = FText::FromString("Explore your home.");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FName HomeExploreObjectiveID = "HomeExplore";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FText HomeFindLightObjectiveText = FText::FromString("Find something to light the way");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FName HomeFindLightObjectiveID = "HomeFindLight";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FText HomeCraftLanternObjectiveText = FText::FromString("Light the lantern");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FName HomeCraftLanternObjectiveID = "HomeCraftLantern";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FText HomeReadyForBoatObjectiveText = FText::FromString("Go to the boat");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FName HomeReadyForBoatObjectiveID = "HomeReadyForBoat";
	
	/******************   ISLAND 1    ********************/
	
	// Handles Island 1 flow
	void UpdateIsland1Flow(AProgressionManager* ProgressionManager);
	
	/** Island 1 Flags **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Flags")
	FName TalkedToListenerIntroFlag = "TalkedToListenerIntro";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Flags")
	FName SomeMelodyPiecesFoundFlag = "SomeMelodyPiecesFound";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Flags")
	FName AllMelodyPiecesFoundFlag = "AllMelodyPiecesFound";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Flags")
	FName Island1PuzzleSolvedFlag = "Island1PuzzleSolved";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Flags")
	FName TalkedToListenerAfterPuzzleFlag = "TalkedToListenerAfterPuzzle";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Flags")
	TArray<FName> MelodyPieceFlags = {
		"NotePiece1",
		"NotePiece2",
		"NotePiece3",
		"NotePiece4"
	};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 1")
	FName ShellReceivedFromIsland1Flag = "ShellReceivedFromIsland1";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 1")
	FName Island2UnlockedFlag = "Island2Unlocked";
	
	/** Island 1 Objectives **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FText Island1ExploreObjectiveText = FText::FromString("Explore Island 1.");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FName Island1ExploreObjectiveID = "ExploreIsland1";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FText FindMelodyPiecesObjectiveText = FText::FromString("Find the missing melody pieces.");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FName FindMelodyPiecesObjectiveID = "FindMelodyPiecesObjective";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FText ReturnToListenerObjectiveText = FText::FromString("Return to The Listener.");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FName ReturnToListenerObjectiveID = "ReturnToListenerObjective";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FText ReturnToBoatObjectiveText = FText::FromString("Return to Boat.");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro|Objectives")
	FName ReturnToBoatObjectiveID = "ReturnToBoatObjective";
	
	// Helpers Island1
	// Returns true if all melody piece flags are collected.
	bool AreAllMelodyPiecesFound(AProgressionManager* ProgressionManager) const;

	//Returns true if at least one melody piece has been collected.
	bool HasAnyMelodyPiece(AProgressionManager* ProgressionManager) const;

	
	
	/******************   ISLAND 2    ********************/
	
	// Handles Island 2 flow
	void UpdateIsland2Flow(AProgressionManager* ProgressionManager);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2")
	FName ArrivedIsland2Flag = "ArrivedIsland2";
	
	/** Island 2 Flags **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2")
	FName TalkedToIsland2NPCIntroFlag = "TalkedToIsland2NPCIntro";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island2")
	FName RustyCrankPickedUpFlag = "RustyCrankPickedUp";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2")
	FName SmallGearPickedUpFlag = "SmallGearPickedUp";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2")
	FName AllGramophonePartsFoundFlag = "AllGramophonePartsFound";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2")
	FName ShellAttachedToGramophoneFlag = "ShellAttachedToGramophone";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2")
	FName GramophoneMechanismCraftedFlag = "GramophoneMechanismCrafted";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2")
	FName GramophoneReadyFlag = "GramophoneReady";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2")
	FName Island2NPCExitedHouseFlag = "Island2NPCExitedHouse";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2")
	FName TalkedToIsland2NPCAfterMusicFlag = "TalkedToIsland2NPCAfterMusic";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story FLow|Island 2")
	FName PenReceivedFlag = "PenReceived";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story FLow|Island 2")
	FName Island2PuzzleSolvedFlag = "Island2PuzzleSolved";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2")
	FName Island3UnlockedFlag = "Island3Unlocked";
	
	/** Island 2 Objectives **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2 Objectives")
	FText Island2ExploreObjectiveText = FText::FromString("Explore Island 2.");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2 Objectives")
	FName Island2ExploreObjectiveID = "ExploreIsland2";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2 Objectives")
	FText Island2FindPartsObjectiveText = FText::FromString("Find parts to repair the gramophone.");
	
	
	
	
	
	
	
	
	
	/******************* UNCOMMENTED CODE! DELETE LATER!! *******************/
	
	/*
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
	
	*/

	/*
	//Starting objective for Island 1.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FText InitialObjectiveText = FText::FromString("Find the missing melody pieces.");

	//Starting objective ID for Island 1.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FName InitialObjectiveID = "FindMelodyPieces";
*/
	
	/*
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
	*/
	
	
	/***********************   NEXT LEVEL???    ****************************/
};
