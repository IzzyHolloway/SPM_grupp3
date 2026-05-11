// Fill out your copyright notice in the Description page of Project Settings.
#include "StoryFlowManager.h"
#include "ProgressionManager.h"
#include "InventoryComponent.h"
#include "GameFramework/Character.h"
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

	if (ProgressionManager->HasFlag(ArrivedIsland2Flag))
	{
		SetStoryState(EStoryState::Island2_TalkToNPCInside);
	}
	else if (ProgressionManager->HasFlag(ArrivedIsland1Flag))
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

	// Global reward check.
	// This must happen before choosing island flow.
	if (ProgressionManager->HasFlag(TalkedToListenerAfterPuzzleFlag))
	{
		if (!ProgressionManager->HasFlag(ShellReceivedFromIsland1Flag))
		{
			ProgressionManager->AddFlag(ShellReceivedFromIsland1Flag);
		}

		TryAddShellToInventory(ProgressionManager);
	}

	// Check Island 2 first, because the player may still have ArrivedIsland1 from earlier.
	if (ProgressionManager->HasFlag(ArrivedIsland2Flag))
	{
		UpdateIsland2Flow(ProgressionManager);
		return;
	}

	if (ProgressionManager->HasFlag(ArrivedIsland1Flag))
	{
		UpdateIsland1Flow(ProgressionManager);
		return;
	}

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

	if (bHasLitLantern)
	{
		if (!ProgressionManager->HasFlag(HomeReadyForBoatFlag))
		{
			ProgressionManager->AddFlag(HomeReadyForBoatFlag);
		}

		SetStoryState(EStoryState::Home_ReadyForBoat);
		return;
	}

	if (bHasLantern && bHasMatches)
	{
		SetStoryState(EStoryState::Home_CraftLantern);
		return;
	}

	if (bHasLantern || bHasMatches)
	{
		SetStoryState(EStoryState::Home_FindLight);
		return;
	}

	SetStoryState(EStoryState::Home_Explore);
}

void AStoryFlowManager::UpdateIsland1Flow(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager)
	{
		return;
	}

	if (ProgressionManager->HasFlag(TalkedToListenerAfterPuzzleFlag))
	{
		if (!ProgressionManager->HasFlag(ShellReceivedFromIsland1Flag))
		{
			ProgressionManager->AddFlag(ShellReceivedFromIsland1Flag);
		}

		TryAddShellToInventory(ProgressionManager);
		
		if (!ProgressionManager->HasFlag(Island2UnlockedFlag))
		{
			ProgressionManager->AddFlag(Island2UnlockedFlag);
		}

		SetStoryState(EStoryState::Island1_ReadyToLeave);
		return;
	}

	if (ProgressionManager->HasFlag(Island1PuzzleSolvedFlag))
	{
		SetStoryState(EStoryState::Island1_PuzzleSolved);
		return;
	}

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
		return;
	}

	if (HasAnyMelodyPiece(ProgressionManager))
	{
		if (!ProgressionManager->HasFlag(SomeMelodyPiecesFoundFlag))
		{
			ProgressionManager->AddFlag(SomeMelodyPiecesFoundFlag);
		}

		SetStoryState(EStoryState::Island1_CollectMelodyPieces);
		return;
	}

	if (ProgressionManager->HasFlag(TalkedToListenerIntroFlag))
	{
		SetStoryState(EStoryState::Island1_CollectMelodyPieces);
		return;
	}

	SetStoryState(EStoryState::Island1_Explore);
}

void AStoryFlowManager::UpdateIsland2Flow(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager)
	{
		return;
	}

	const bool bTalkedToNPCIntro = ProgressionManager->HasFlag(Island2NPCIntroTalkedFlag);
	const bool bHasCrank = ProgressionManager->HasFlag(RustyCrankPickedUpFlag);
	const bool bHasGear = ProgressionManager->HasFlag(SmallGearPickedUpFlag);
	const bool bHasShell = ProgressionManager->HasFlag(ShellReceivedFromIsland1Flag);

	const bool bAllCraftIngredientsFound = bHasCrank && bHasGear && bHasShell;

	const bool bMechanismCrafted = ProgressionManager->HasFlag(GramophoneMechanismCraftedFlag);
	const bool bMechanismInstalled = ProgressionManager->HasFlag(GramophoneMechanismInstalledFlag);
	const bool bGramophonePlayed = ProgressionManager->HasFlag(GramophonePlayedFlag);
	const bool bTalkedAfterMusic = ProgressionManager->HasFlag(TalkedToIsland2NPCAfterMusicFlag);
	const bool bIsland2Solved = ProgressionManager->HasFlag(Island2PuzzleSolvedFlag);

	if (bIsland2Solved)
	{
		if (!ProgressionManager->HasFlag(Island3UnlockedFlag))
		{
			ProgressionManager->AddFlag(Island3UnlockedFlag);
		}

		SetStoryState(EStoryState::Island2_ReadyToLeave);
		return;
	}

	if (bTalkedAfterMusic)
	{
		if (!ProgressionManager->HasFlag(PenReceivedFromIsland2NPCFlag))
		{
			ProgressionManager->AddFlag(PenReceivedFromIsland2NPCFlag);
		}

		if (!ProgressionManager->HasFlag(Island2PuzzleSolvedFlag))
		{
			ProgressionManager->AddFlag(Island2PuzzleSolvedFlag);
		}

		if (!ProgressionManager->HasFlag(Island3UnlockedFlag))
		{
			ProgressionManager->AddFlag(Island3UnlockedFlag);
		}

		SetStoryState(EStoryState::Island2_ReadyToLeave);
		return;
	}

	if (bGramophonePlayed)
	{
		if (!ProgressionManager->HasFlag(Island2NPCExitedHouseFlag))
		{
			ProgressionManager->AddFlag(Island2NPCExitedHouseFlag);
		}

		SetStoryState(EStoryState::Island2_ReturnToNPC);
		return;
	}

	if (bMechanismInstalled)
	{
		SetStoryState(EStoryState::Island2_PlayGramophone);
		return;
	}

	if (bMechanismCrafted)
	{
		SetStoryState(EStoryState::Island2_InstallMechanism);
		return;
	}

	if (bAllCraftIngredientsFound)
	{
		if (!ProgressionManager->HasFlag(AllGramophonePartsFoundFlag))
		{
			ProgressionManager->AddFlag(AllGramophonePartsFoundFlag);
		}

		SetStoryState(EStoryState::Island2_CraftMechanism);
		return;
	}

	if (HasAnyGramophonePart(ProgressionManager) || bTalkedToNPCIntro)
	{
		SetStoryState(EStoryState::Island2_FindGramophoneParts);
		return;
	}

	SetStoryState(EStoryState::Island2_TalkToNPCInside);
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

bool AStoryFlowManager::AreAllGramophonePartsFound(AProgressionManager* ProgressionManager) const
{
	if (!ProgressionManager)
	{
		return false;
	}

	return ProgressionManager->HasFlag(RustyCrankPickedUpFlag)
		&& ProgressionManager->HasFlag(SmallGearPickedUpFlag)
		&& ProgressionManager->HasFlag(ShellReceivedFromIsland1Flag);
}

bool AStoryFlowManager::HasAnyGramophonePart(AProgressionManager* ProgressionManager) const
{
	if (!ProgressionManager)
	{
		return false;
	}

	return ProgressionManager->HasFlag(RustyCrankPickedUpFlag)
		|| ProgressionManager->HasFlag(SmallGearPickedUpFlag);
}



void AStoryFlowManager::TryAddShellToInventory(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager)
	{
		return;
	}

	// Do not add the shell twice.
	if (ProgressionManager->HasFlag(ShellItemAddedToInventoryFlag))
	{
		return;
	}

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not add shell: No player character found."));
		return;
	}

	UInventoryComponent* InventoryComponent = PlayerCharacter->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not add shell: No InventoryComponent found."));
		return;
	}

	const bool bAdded = InventoryComponent->AddItemToInventory(ShellItemID, 1);

	if (bAdded)
	{
		ProgressionManager->AddFlag(ShellItemAddedToInventoryFlag);

		UE_LOG(LogTemp, Warning, TEXT("Shell added to inventory."));
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.f,
				FColor::Green,
				TEXT("Shell added to inventory")
			);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not add shell: Inventory rejected item."));
	}
}


void AStoryFlowManager::TryAddPenToInventory(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager)
	{
		return;
	}

	// Do not add the shell twice.
	if (ProgressionManager->HasFlag(PenItemAddedToInventoryFlag))
	{
		return;
	}

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not add pen: No player character found."));
		return;
	}

	UInventoryComponent* InventoryComponent = PlayerCharacter->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not add pen: No InventoryComponent found."));
		return;
	}

	const bool bAdded = InventoryComponent->AddItemToInventory(PenItemID, 1);

	if (bAdded)
	{
		ProgressionManager->AddFlag(PenItemAddedToInventoryFlag);

		UE_LOG(LogTemp, Warning, TEXT("Pen added to inventory."));
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.f,
				FColor::Green,
				TEXT("Pen added to inventory")
			);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not add pen: Inventory rejected item."));
	}
}