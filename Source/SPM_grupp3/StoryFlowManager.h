// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoryFlowManager.generated.h"

class AProgressionManager;

/*
 * Story States
 *
 * Big event: "Where are we in the story?"
 * Smaller details are handled with progression flags.
 */

UENUM(BlueprintType)
enum class EStoryState : uint8
{
	// INTRO / Home
	Home_Explore UMETA(DisplayName = "Home - Explore"),
	Home_FindLight UMETA(DisplayName = "Home - Find Light"),
	Home_CraftLantern UMETA(DisplayName = "Home - Craft Lantern"),
	Home_ReadyForBoat UMETA(DisplayName = "Home - Ready for Boat"),

	// Island 1
	Island1_Explore UMETA(DisplayName = "Island 1 - Explore"),
	Island1_CollectMelodyPieces UMETA(DisplayName = "Island 1 - Collect Melody Pieces"),
	Island1_ReturnToListener UMETA(DisplayName = "Island 1 - Return to Listener"),
	Island1_PuzzleSolved UMETA(DisplayName = "Island 1 - Puzzle Solved"),
	Island1_ReadyToLeave UMETA(DisplayName = "Island 1 - Ready to Leave"),

	// Island 2
	Island2_TalkToNPCInside UMETA(DisplayName = "Island 2 - Talk to NPC Inside"),
	Island2_FindGramophoneParts UMETA(DisplayName = "Island 2 - Find Gramophone Parts"),
	Island2_CraftMechanism UMETA(DisplayName = "Island 2 - Craft Mechanism"),
	Island2_InstallMechanism UMETA(DisplayName = "Island 2 - Install Mechanism"),
	//Island2_AttachShell UMETA(DisplayName = "Island 2 - Attach Shell"),
	Island2_PlayGramophone UMETA(DisplayName = "Island 2 - Play Gramophone"),
	Island2_ReturnToNPC UMETA(DisplayName = "Island 2 - Return to NPC"),
	Island2_ReadyToLeave UMETA(DisplayName = "Island 2 - Ready to Leave"),
	
	// Island 3
	Island3_TalkToNPC UMETA(DisplayName = "Island 3 - Talk to NPC "),
	Island3_FindPaper UMETA(DisplayName = "Island 3 - Find Paper"),
	Island3_CraftNote UMETA(DisplayName = "Island 3 - Craft Drawing"),
	Island3_GiveNoteToNPC UMETA(DisplayName = "Island 3 - Give the drawing to NPC"),
	Island3_EnterHouse UMETA(DisplayName = "Island 3 - Enter the house"),
	Island3_OpenPadlock UMETA(DisplayName = "Island 3 - Open the Padlock"),
	Island3_OpenGate UMETA(DisplayName = "Island 3 - Open the gate to Level2"),
	Island3_ReadyToLeave UMETA(DisplayName = "Island 3 - Leave Level1"),
	
	// Level2
	Level2_ChooseIsland UMETA(DisplayName = "Level 2 - Choose Island"),
	
	Level2_MotorIslandExplore UMETA(DisplayName = "Level 2 - Motor Island Explore"),
	Level2_MotorIslandFindParts UMETA(DisplayName = "Level 2 - Motor Island Find Parts"),
	Level2_MotorIslandCraftHalf UMETA(DisplayName = "Level 2 - Motor Island Craft Half"),
	Level2_MotorIslandSolved UMETA(DisplayName = "Level 2 - Motor Island Solved"),
	
	Level2_LeverIslandExplore UMETA(DisplayName = "Level 2 - Lever Island Explore"),
	Level2_LeverIslandFindParts UMETA(DisplayName = "Level 2 - Lever Island Find Parts"),
	Level2_LeverIslandCraftHalf UMETA(DisplayName = "Level 2 - Lever Island Craft Half"),
	Level2_LeverIslandSolved UMETA(DisplayName = "Level 2 - Lever Island Solved"),
	
	Level2_FinalCraftAvailable UMETA(DisplayName = "Level 2 - Final Craft Available"),
	Level2_FinalItemCrafted UMETA(DisplayName = "Level 2 - Final Item Crafted"),
	
	Lighthouse_NotReady UMETA(DisplayName = "Lighthouse - NotReady"),
	Lighthouse_Ready UMETA(DisplayName = "Lighthouse - Ready"),
	Lighthouse_Ending UMETA(DisplayName = "Lighthouse - Ending"),
	
};

/**
 * Central controller for story and progression flow.
 *
 * Responsibilities:
 * - Watch progression flags.
 * - Set the current story state.
 * - Add automatic progression flags when important milestones are reached.
 *
 * Dialogue, pickups, crafting, boat blocking, and puzzle interactables
 * should still use ProgressionManager flags directly.
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

	void UpdateStoryFlow();
	void SetStoryState(EStoryState NewState);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow")
	EStoryState CurrentStoryState = EStoryState::Home_Explore;

	/****************** HOME / INTRO ********************/

	void UpdateHomeFlow(AProgressionManager* ProgressionManager);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Home|Flags")
	FName PickedUpLanternFlag = "PickedUpLantern";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Home|Flags")
	FName PickedUpMatchesFlag = "PickUpMatches";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Home|Flags")
	FName LitLanternFlag = "LitLantern";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Home|Flags")
	FName HomeReadyForBoatFlag = "HomeReadyForBoat";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Travel|Flags")
	FName ArrivedIsland1Flag = "ArrivedIsland1";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Travel|Flags")
	FName ArrivedIsland2Flag = "ArrivedIsland2";

	/****************** ISLAND 1 ********************/

	void UpdateIsland1Flow(AProgressionManager* ProgressionManager);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 1|Flags")
	FName TalkedToListenerIntroFlag = "TalkedToListenerIntro";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 1|Flags")
	FName SomeMelodyPiecesFoundFlag = "SomeMelodyPiecesFound";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 1|Flags")
	FName AllMelodyPiecesFoundFlag = "AllMelodyPiecesFound";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 1|Flags")
	FName Island1PuzzleSolvedFlag = "Island1PuzzleSolved";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 1|Flags")
	FName TalkedToListenerAfterPuzzleFlag = "TalkedToListenerAfterPuzzle";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 1|Flags")
	TArray<FName> MelodyPieceFlags =
	{
		"NotePiece1",
		"NotePiece2",
		"NotePiece3",
		"NotePiece4"
	};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 1|Flags")
	FName ShellReceivedFromIsland1Flag = "ShellReceivedFromIsland1";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 1|Flags")
	FName Island2UnlockedFlag = "Island2Unlocked";

	bool AreAllMelodyPiecesFound(AProgressionManager* ProgressionManager) const;
	bool HasAnyMelodyPiece(AProgressionManager* ProgressionManager) const;

	/****************** ISLAND 2 ********************/

	void UpdateIsland2Flow(AProgressionManager* ProgressionManager);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName Island2NPCIntroTalkedFlag = "Island2NPCIntroTalked";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName RustyCrankPickedUpFlag = "RustyCrankPickedUp";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName SmallGearPickedUpFlag = "SmallGearPickedUp";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName AllGramophonePartsFoundFlag = "AllGramophonePartsFound";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName GramophoneMechanismCraftedFlag = "GramophoneMechanismCrafted";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName GramophoneMechanismInstalledFlag = "GramophoneMechanismInstalled";
/*
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName ShellAttachedToGramophoneFlag = "ShellAttachedToGramophone";
*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName GramophoneReadyFlag = "GramophoneReady";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName GramophonePlayedFlag = "GramophonePlayed";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName Island2NPCExitedHouseFlag = "Island2NPCExitedHouse";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName TalkedToIsland2NPCAfterMusicFlag = "TalkedToIsland2NPCAfterMusic";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName PenReceivedFromIsland2NPCFlag = "PenReceivedFromIsland2NPC";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName Island2PuzzleSolvedFlag = "Island2PuzzleSolved";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2|Flags")
	FName Island3UnlockedFlag = "Island3Unlocked";

	bool AreAllGramophonePartsFound(AProgressionManager* ProgressionManager) const;
	bool HasAnyGramophonePart(AProgressionManager* ProgressionManager) const;
	
	/****************** ISLAND 3 ********************/
	
	void UpdateIsland3Flow(AProgressionManager* ProgressionManager);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Island 3")
	FName ArrivedIsland3Flag = "ArrivedIsland3";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Island 3")
	FName Island3NPCIntroTalkedFlag = "Island3NPCIntroTalked";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Island 3")
	FName Island3PaperPickedUpFlag = "Island3PaperPickedUp";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Island 3")
	FName Island3NoteCraftedFlag = "Island3NoteCrafted";

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Island 3")
	//FName Island3NoteGivenToNPCFlag = "Island3NoteGivenToNPC";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Island 3")
	FName Island3HouseAccessAllowedFlag = "Island3HouseAccessAllowed";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Island 3")
	FName Island3PadlockSolvedFlag = "Island3PadlockSolved";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Island 3")
	FName Island3GateOpenedFlag = "Island3GateOpened";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Island 3")
	FName Island3PuzzleSolvedFlag = "Island3PuzzleSolved";
	
	
	/****************** LEVEL 2 ********************/

	void UpdateLevel2Flow(AProgressionManager* ProgressionManager);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Level 2")
	FName ArrivedLevel2Flag = "ArrivedLevel2";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Level 2")
	FName ArrivedLevel2MotorIslandFlag = "ArrivedLevel2MotorIsland";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Level 2")
	FName ArrivedLevel2LeverIslandFlag = "ArrivedLevel2LeverIsland";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Level 2")
	FName ArrivedLighthouseIslandFlag = "ArrivedLighthouseIsland";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Level 2")
	FName Level2MotorIslandSolvedFlag = "Level2MotorIslandSolved";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Level 2")
	FName Level2LeverIslandSolvedFlag = "Level2LeverIslandSolved";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Level 2")
	FName Level2FinalItemCraftedFlag = "Level2FinalItemCrafted";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Level 2")
	FName LifeCompassReceivedFlag = "LifeCompassReceived";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Level 2")
	FName LighthouseReadyFlag = "LighthouseReady";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Level 2")
	FName LighthouseLitFlag = "LighthouseLit";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flags|Level 2")
	FName GameEndingStartedFlag = "GameEndingStarted";
	
	
	/*** HARD CODED!!!! REMOVE LATER!!!! ***/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 1")
	FName ShellItemAddedToInventoryFlag = "ShellItemAddedToInventory";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 1")
	FName ShellItemID = "Shell";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2")
	FName PenItemAddedToInventoryFlag = "PenItemAddedToInventory";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Flow|Island 2")
	FName PenItemID = "Pen";
	
	void TryAddShellToInventory(AProgressionManager* ProgressionManager);
	
	void TryAddPenToInventory(AProgressionManager* ProgressionManager);
};