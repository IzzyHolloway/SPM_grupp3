// Fill out your copyright notice in the Description page of Project Settings.
#include "StoryFlowManager.h"
#include "DialogueManager.h"
#include "ObjectiveManager.h"
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

	CacheRuntimeReferences();

	AProgressionManager* ProgressionManager = GetProgressionManager();

	if (!ProgressionManager)
	{
		return;
	}
	
	if (IsInLevel2(ProgressionManager))
	{
		SetStoryState(EStoryState::Level2_ChooseIsland);
	}
	else if (ProgressionManager->HasFlag(ArrivedIsland3Flag))
	{
		SetStoryState(EStoryState::Island3_TalkToNPC);
	}
	else if (ProgressionManager->HasFlag(ArrivedIsland2Flag))
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

	/*
	if (ProgressionManager->HasFlag(ArrivedIsland3Flag))
	{
		SetStoryState(EStoryState::Island3_TalkToNPC);
	}
	else if (ProgressionManager->HasFlag(ArrivedIsland2Flag))
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
	*/
	
	if (UInventoryComponent* InventoryComponent = GetInventoryComponent())
	{
		InventoryComponent->OnItemPickedUp.AddDynamic(this, &AStoryFlowManager::HandleItemPickedUp);
	}

	UpdateStoryFlow();
	UpdateObjectiveSystems(CurrentStoryState);
}

void AStoryFlowManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateStoryFlow();
}

void AStoryFlowManager::RefreshFromProgression()
{
	CacheRuntimeReferences();
	UpdateStoryFlow();
	UpdateObjectiveSystems(CurrentStoryState);
}

void AStoryFlowManager::CacheRuntimeReferences()
{
	CachedProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	CachedDialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	ObjectiveManager = Cast<AObjectiveManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AObjectiveManager::StaticClass())
	);

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	CachedInventoryComponent = PlayerCharacter ? PlayerCharacter->FindComponentByClass<UInventoryComponent>() : nullptr;
}

AProgressionManager* AStoryFlowManager::GetProgressionManager()
{
	if (!CachedProgressionManager)
	{
		CachedProgressionManager = Cast<AProgressionManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
		);
	}

	return CachedProgressionManager;
}

ADialogueManager* AStoryFlowManager::GetDialogueManager()
{
	if (!CachedDialogueManager)
	{
		CachedDialogueManager = Cast<ADialogueManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
		);
	}

	return CachedDialogueManager;
}

UInventoryComponent* AStoryFlowManager::GetInventoryComponent()
{
	if (!CachedInventoryComponent)
	{
		ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		CachedInventoryComponent = PlayerCharacter ? PlayerCharacter->FindComponentByClass<UInventoryComponent>() : nullptr;
	}

	return CachedInventoryComponent;
}

bool AStoryFlowManager::IsInLevel2(AProgressionManager* ProgressionManager) const
{
	return ProgressionManager &&
		(ProgressionManager->HasFlag(ArrivedLevel2Flag) ||
		ProgressionManager->HasFlag(ArrivedLevel2MotorIslandFlag) ||
		ProgressionManager->HasFlag(ArrivedLevel2LeverIslandFlag) ||
		ProgressionManager->HasFlag(ArrivedLighthouseIslandFlag));
}

bool AStoryFlowManager::TryShowOneShotMessage(AProgressionManager* ProgressionManager, FName MessageShownFlag, const FText& Message, bool bBlockOnAnyVisibleDialogue)
{
	if (!ProgressionManager || ProgressionManager->HasFlag(MessageShownFlag))
	{
		return false;
	}

	ADialogueManager* DialogueManager = GetDialogueManager();
	if (!DialogueManager)
	{
		return false;
	}

	const bool bDialogueBlocked = bBlockOnAnyVisibleDialogue
		? DialogueManager->IsDialogueOrMessageVisible()
		: DialogueManager->IsDialogueActive();

	if (bDialogueBlocked)
	{
		return false;
	}

	ProgressionManager->AddFlag(MessageShownFlag);
	DialogueManager->ShowMessage(Message);
	return true;
}

void AStoryFlowManager::TryAddItemToInventoryOnce(AProgressionManager* ProgressionManager, FName ItemID, FName AddedFlag, const TCHAR* DebugName)
{
	if (!ProgressionManager || ItemID.IsNone() || ProgressionManager->HasFlag(AddedFlag))
	{
		return;
	}

	UInventoryComponent* InventoryComponent = GetInventoryComponent();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not add %s: No InventoryComponent found."), DebugName);
		return;
	}

	const bool bAdded = InventoryComponent->AddItemToInventory(ItemID, 1);
	if (bAdded)
	{
		ProgressionManager->AddFlag(AddedFlag);
		UE_LOG(LogTemp, Warning, TEXT("%s added to inventory."), DebugName);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Could not add %s: Inventory rejected item."), DebugName);
}

void AStoryFlowManager::TryClearInventoryItemsOnce(AProgressionManager* ProgressionManager, const TArray<FName>& ItemIDs, FName ClearedFlag, const TCHAR* DebugName)
{
	if (!ProgressionManager || ProgressionManager->HasFlag(ClearedFlag))
	{
		return;
	}

	UInventoryComponent* InventoryComponent = GetInventoryComponent();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not clear %s items: No InventoryComponent."), DebugName);
		return;
	}

	for (const FName& ItemID : ItemIDs)
	{
		if (ItemID.IsNone())
		{
			continue;
		}

		const bool bRemoved = InventoryComponent->RemoveItemByID(ItemID);
		UE_LOG(LogTemp, Warning, TEXT("%s cleanup: %s -> %s"),
			DebugName,
			*ItemID.ToString(),
			bRemoved ? TEXT("removed") : TEXT("not found"));
	}

	ProgressionManager->AddFlag(ClearedFlag);
}

void AStoryFlowManager::UpdateStoryFlow()
{
	AProgressionManager* ProgressionManager = GetProgressionManager();

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
	
	if (IsInLevel2(ProgressionManager))
	{
		SetStoryState(EStoryState::Level2_ChooseIsland);
	}
	
	// Check Level 2 first, because old Island 1/2/3 arrival flags may still exist.
	if (IsInLevel2(ProgressionManager))
	{
		UpdateLevel2Flow(ProgressionManager);
		return;
	}
	
	if (ProgressionManager->HasFlag(ArrivedIsland3Flag))
	{
		UpdateIsland3Flow(ProgressionManager);
		return;
	}
	
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
		TryShowOneShotMessage(ProgressionManager, HomeCraftLanternDialogueShownFlag, HomeCraftLanternMessage, true);

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

	TryClearHomeItemsFromInventory(ProgressionManager);

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
	
	// Delayed hint about placing in order
	if (ProgressionManager->HasFlag(TalkedToListenerAfterAllMelodyPiecesFlag) &&
	!ProgressionManager->HasFlag(Island1PuzzleSolvedFlag))
	{
		TryScheduleMelodyOrderHint(ProgressionManager);

		SetStoryState(EStoryState::Island1_ReturnToListener); // or better: add a new state later
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

		TryShowOneShotMessage(ProgressionManager, Island1AllItemsDialogueShownFlag, Island1AllItemsFoundMessage, true);

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

	TryClearIsland1ItemsFromInventory(ProgressionManager);

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

		TryAddPenToInventory(ProgressionManager);

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

		TryShowOneShotMessage(ProgressionManager, Island2AllItemsDialogueShownFlag, Island2AllItemsFoundMessage, false);

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

void AStoryFlowManager::UpdateIsland3Flow(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager)
	{
		return;
	}

	TryClearIsland2ItemsFromInventory(ProgressionManager);

	const bool bTalkedToNPC = ProgressionManager->HasFlag(Island3NPCIntroTalkedFlag);
	const bool bHasPaper = ProgressionManager->HasFlag(Island3PaperPickedUpFlag);
	const bool bHasPen = ProgressionManager->HasFlag(PenItemAddedToInventoryFlag);
	const bool bNoteCrafted = ProgressionManager->HasFlag(Island3NoteCraftedFlag);
	//const bool bNoteGivenToNPC = ProgressionManager->HasFlag(Island3NoteGivenToNPCFlag);
	const bool bHouseAccessAllowed = ProgressionManager->HasFlag(Island3HouseAccessAllowedFlag);
	const bool bPadlockSolved = ProgressionManager->HasFlag(Island3PadlockSolvedFlag);
	const bool bGateOpened = ProgressionManager->HasFlag(Island3GateOpenedFlag);

	if (bGateOpened)
	{
		if (!ProgressionManager->HasFlag(Island3PuzzleSolvedFlag))
		{
			ProgressionManager->AddFlag(Island3PuzzleSolvedFlag);
		}

		SetStoryState(EStoryState::Island3_ReadyToLeave);
		return;
	}

	if (bPadlockSolved)
	{
		SetStoryState(EStoryState::Island3_OpenGate);
		return;
	}

	if (bHouseAccessAllowed)
	{
		SetStoryState(EStoryState::Island3_OpenPadlock);
		return;
	}

	/*
	if (bNoteGivenToNPC)
	{
		if (!ProgressionManager->HasFlag(Island3HouseAccessAllowedFlag))
		{
			ProgressionManager->AddFlag(Island3HouseAccessAllowedFlag);
		}

		SetStoryState(EStoryState::Island3_EnterHouse);
		return;
	}
	*/

	if (bNoteCrafted)
	{
		SetStoryState(EStoryState::Island3_GiveNoteToNPC);
		return;
	}

	if (bHasPaper && bHasPen)
	{
		TryShowOneShotMessage(ProgressionManager, Island3AllItemsDialogueShownFlag, Island3AllItemsFoundMessage, false);

		SetStoryState(EStoryState::Island3_CraftNote);
		return;
	}

	if (bTalkedToNPC)
	{
		SetStoryState(EStoryState::Island3_FindPaper);
		return;
	}

	SetStoryState(EStoryState::Island3_TalkToNPC);
}

void AStoryFlowManager::UpdateLevel2Flow(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager)
	{
		return;
	}

	TryClearIsland3ItemsFromInventory(ProgressionManager);

	const bool bArrivedMotorIsland = ProgressionManager->HasFlag(ArrivedLevel2MotorIslandFlag);
	const bool bArrivedLeverIsland = ProgressionManager->HasFlag(ArrivedLevel2LeverIslandFlag);
	const bool bArrivedLighthouseIsland = ProgressionManager->HasFlag(ArrivedLighthouseIslandFlag);

	const bool bMotorIslandSolved = ProgressionManager->HasFlag(Level2MotorIslandSolvedFlag);
	const bool bLeverIslandSolved = ProgressionManager->HasFlag(Level2LeverIslandSolvedFlag);
	const bool bFinalItemCrafted = ProgressionManager->HasFlag(Level2FinalItemCraftedFlag);

	const bool bCompassBearerSpawned = ProgressionManager->HasFlag(CompassBearerSpawnedFlag);
	const bool bTalkedToCompassBearer = ProgressionManager->HasFlag(TalkedToCompassBearerFlag);
	const bool bLifeCompassReceived = ProgressionManager->HasFlag(LifeCompassReceivedFlag);

	const bool bTalkedToEntity = ProgressionManager->HasFlag(TalkedToLighthouseEntityFlag);
	const bool bEnteredLighthouse = ProgressionManager->HasFlag(EnteredLighthouseFlag);
	const bool bExploredLighthouse = ProgressionManager->HasFlag(LighthouseExploredFlag);
	const bool bEngineInstalled = ProgressionManager->HasFlag(LighthouseEngineInstalledFlag);
	const bool bLightCutscenePlayed = ProgressionManager->HasFlag(LighthouseLightCutscenePlayedFlag);
	const bool bTalkedAfterLight = ProgressionManager->HasFlag(TalkedAfterLighthouseLightFlag);
	const bool bFinalCutsceneStarted = ProgressionManager->HasFlag(FinalCutsceneStartedFlag);

	auto ShowCompassBearer = [this]()
	{
		if (!CompassBearerActorClass)
		{
			return;
		}

		AActor* CompassBearerActor = UGameplayStatics::GetActorOfClass(GetWorld(), CompassBearerActorClass);
		if (!CompassBearerActor)
		{
			return;
		}

		static const FName TriggerEventName(TEXT("ShowCompassBearer"));
		if (UFunction* TriggerFunction = CompassBearerActor->FindFunction(TriggerEventName))
		{
			CompassBearerActor->ProcessEvent(TriggerFunction, nullptr);
		}
	};

	// -------------------------------
	// Lighthouse ending part
	// Check latest steps first.
	// -------------------------------

	if (bFinalCutsceneStarted)
	{
		if (!ProgressionManager->HasFlag(GameEndingStartedFlag))
		{
			ProgressionManager->AddFlag(GameEndingStartedFlag);
		}

		SetStoryState(EStoryState::Lighthouse_EndingCutscene);
		return;
	}

	if (bTalkedAfterLight)
	{
		SetStoryState(EStoryState::Lighthouse_EndingCutscene);
		return;
	}

	if (bLightCutscenePlayed)
	{
		SetStoryState(EStoryState::Lighthouse_AfterLightDialogue);
		return;
	}

	if (bEngineInstalled)
	{
		TryClearLighthouseItemsFromInventory(ProgressionManager);
		SetStoryState(EStoryState::Lighthouse_LightCutscene);
		return;
	}

	if (bExploredLighthouse)
	{
		SetStoryState(EStoryState::Lighthouse_InstallEngine);
		return;
	}

	if (bEnteredLighthouse)
	{
		SetStoryState(EStoryState::Lighthouse_Explore);
		return;
	}

	if (bTalkedToEntity)
	{
		SetStoryState(EStoryState::Lighthouse_EnterLighthouse);
		return;
	}

	if (bArrivedLighthouseIsland)
	{
		if (bLifeCompassReceived)
		{
			if (!ProgressionManager->HasFlag(LighthouseReadyFlag))
			{
				ProgressionManager->AddFlag(LighthouseReadyFlag);
			}

			SetStoryState(EStoryState::Lighthouse_TalkToEntity);
			return;
		}

		SetStoryState(EStoryState::Lighthouse_NotReady);
		return;
	}

	// -------------------------------
	// Compass Bearer / Level 2 reward
	// -------------------------------

	if (bLifeCompassReceived)
	{
		if (!ProgressionManager->HasFlag(LighthouseReadyFlag))
		{
			ProgressionManager->AddFlag(LighthouseReadyFlag);
		}

		SetStoryState(EStoryState::Level2_CompassReceived);
		return;
	}

	if (bTalkedToCompassBearer)
	{
		if (!ProgressionManager->HasFlag(LifeCompassReceivedFlag))
		{
			ProgressionManager->AddFlag(LifeCompassReceivedFlag);
		}

		if (!ProgressionManager->HasFlag(LighthouseReadyFlag))
		{
			ProgressionManager->AddFlag(LighthouseReadyFlag);
		}

		SetStoryState(EStoryState::Level2_CompassReceived);
		return;
	}

	if (bCompassBearerSpawned)
	{
		ShowCompassBearer();
		SetStoryState(EStoryState::Level2_CompassBearerSpawned);
		return;
	}

	/*
	if (bFinalItemCrafted)
	{
		SetStoryState(EStoryState::Level2_FinalItemCrafted);
		return;
	}
	*/

	// -------------------------------
	// Two Level 2 islands solved
	// -------------------------------

	if (bMotorIslandSolved && bLeverIslandSolved)
	{
		if (!ProgressionManager->HasFlag(CompassBearerSpawnedFlag))
		{
			ProgressionManager->AddFlag(CompassBearerSpawnedFlag);
			ShowCompassBearer();
		}

		SetStoryState(EStoryState::Level2_CompassBearerSpawned);
		return;
	}

	// -------------------------------
	// Individual island states
	// -------------------------------

	if (bArrivedMotorIsland)
	{
		if (!bMotorIslandSolved &&
			ProgressionManager->HasFlag(CopperPipePickupFlag) &&
			ProgressionManager->HasFlag(HandWheelPipePickupFlag) &&
			!ProgressionManager->HasFlag(Level2MotorIslandAllItemsDialogueShownFlag))
		{
			TryShowOneShotMessage(ProgressionManager, Level2MotorIslandAllItemsDialogueShownFlag, Level2MotorIslandAllItemsFoundMessage, false);
		}

		if (bMotorIslandSolved)
		{
			SetStoryState(EStoryState::Level2_MotorIslandSolved);
			return;
		}

		SetStoryState(EStoryState::Level2_MotorIslandExplore);
		return;
	}

	if (bArrivedLeverIsland)
	{
		if (!bLeverIslandSolved &&
			ProgressionManager->HasFlag(SparkPlugPickupFlag) &&
			ProgressionManager->HasFlag(IgnitionCapPickupFlag) &&
			!ProgressionManager->HasFlag(Level2LeverIslandAllItemsDialogueShownFlag))
		{
			TryShowOneShotMessage(ProgressionManager, Level2LeverIslandAllItemsDialogueShownFlag, Level2LeverIslandAllItemsFoundMessage, false);
		}

		if (bLeverIslandSolved)
		{
			SetStoryState(EStoryState::Level2_LeverIslandSolved);
			return;
		}

		SetStoryState(EStoryState::Level2_LeverIslandExplore);
		return;
	}

	SetStoryState(EStoryState::Level2_ChooseIsland);
}

void AStoryFlowManager::SetStoryState(EStoryState NewState)
{
	if (CurrentStoryState == NewState)
	{
		return;
	}

	CurrentStoryState = NewState;

	UE_LOG(LogTemp, Warning, TEXT("Story state changed to: %s"), *UEnum::GetValueAsString(CurrentStoryState));

	UpdateObjectiveSystems(CurrentStoryState);
}

void AStoryFlowManager::UpdateObjectiveSystems(EStoryState NewState)
{
	AProgressionManager* ProgressionManager = GetProgressionManager();

	if (ProgressionManager)
	{
		if (const UEnum* StoryStateEnum = StaticEnum<EStoryState>())
		{
			const FString StateName = StoryStateEnum->GetNameStringByValue(static_cast<int64>(NewState));
			ProgressionManager->SetCurrentObjectiveID(FName(*StateName));
		}

		ProgressionManager->SetCurrentObjectiveText(GetObjectiveTextForState(NewState));
	}

	if (!ObjectiveManager)
	{
		ObjectiveManager = Cast<AObjectiveManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AObjectiveManager::StaticClass())
		);
	}

	if (!ObjectiveManager)
	{
		return;
	}

	const TArray<FName> HomeCraftFlags = { PickedUpLanternFlag, PickedUpMatchesFlag };
	const TArray<FName> Island2CraftFlags = { ShellReceivedFromIsland1Flag, RustyCrankPickedUpFlag, SmallGearPickedUpFlag };
	const TArray<FName> Island3CraftFlags = { PenItemAddedToInventoryFlag, Island3PaperPickedUpFlag };
	const TArray<FName> Level2MotorCraftFlags = { CopperPipePickupFlag, HandWheelPipePickupFlag };
	const TArray<FName> Level2LeverCraftFlags = { SparkPlugPickupFlag, IgnitionCapPickupFlag };

	switch (NewState)
	{
	case EStoryState::Home_FindLight:
	case EStoryState::Home_CraftLantern:
		if (ProgressionManager)
		{
			ObjectiveManager->ShowObjectiveProgress(
				CountSatisfiedFlags(ProgressionManager, HomeCraftFlags),
				HomeCraftFlags.Num());
		}
		break;

	case EStoryState::Island1_CollectMelodyPieces:
	case EStoryState::Island1_ReturnToListener:
		if (ProgressionManager)
		{
			ObjectiveManager->ShowObjectiveProgress(
				CountSatisfiedFlags(ProgressionManager, MelodyPieceFlags),
				MelodyPieceFlags.Num());
		}
		break;

	case EStoryState::Island2_FindGramophoneParts:
	case EStoryState::Island2_CraftMechanism:
		if (ProgressionManager)
		{
			ObjectiveManager->ShowObjectiveProgress(
				CountSatisfiedFlags(ProgressionManager, Island2CraftFlags),
				Island2CraftFlags.Num());
		}
		break;

	case EStoryState::Island3_FindPaper:
	case EStoryState::Island3_CraftNote:
		if (ProgressionManager)
		{
			ObjectiveManager->ShowObjectiveProgress(
				CountSatisfiedFlags(ProgressionManager, Island3CraftFlags),
				Island3CraftFlags.Num());
		}
		break;

	case EStoryState::Level2_MotorIslandExplore:
	case EStoryState::Level2_MotorIslandFindParts:
	case EStoryState::Level2_MotorIslandCraftHalf:
		if (ProgressionManager)
		{
			ObjectiveManager->ShowObjectiveProgress(
				CountSatisfiedFlags(ProgressionManager, Level2MotorCraftFlags),
				Level2MotorCraftFlags.Num());
		}
		break;

	case EStoryState::Level2_LeverIslandExplore:
	case EStoryState::Level2_LeverIslandFindParts:
	case EStoryState::Level2_LeverIslandCraftHalf:
		if (ProgressionManager)
		{
			ObjectiveManager->ShowObjectiveProgress(
				CountSatisfiedFlags(ProgressionManager, Level2LeverCraftFlags),
				Level2LeverCraftFlags.Num());
		}
		break;

	default:
		ObjectiveManager->HideObjective();
		break;
	}
}

int32 AStoryFlowManager::CountSatisfiedFlags(AProgressionManager* ProgressionManager, const TArray<FName>& Flags) const
{
	if (!ProgressionManager)
	{
		return 0;
	}

	int32 Count = 0;
	for (const FName& Flag : Flags)
	{
		if (!Flag.IsNone() && ProgressionManager->HasFlag(Flag))
		{
			++Count;
		}
	}

	return Count;
}

FText AStoryFlowManager::GetObjectiveTextForState(EStoryState State) const
{
	switch (State)
	{
	case EStoryState::Home_Explore:
		return FText::FromString("Look around the house.");
	case EStoryState::Home_FindLight:
		return FText::FromString("Find something to light the lantern.");
	case EStoryState::Home_CraftLantern:
		return FText::FromString("Craft the lantern.");
	case EStoryState::Home_ReadyForBoat:
		return FText::FromString("Go to the boat.");

	case EStoryState::Island1_Explore:
		return FText::FromString("Talk to the Listener.");
	case EStoryState::Island1_CollectMelodyPieces:
		return FText::FromString("Find the melody pieces.");
	case EStoryState::Island1_ReturnToListener:
		return FText::FromString("Return to the Listener.");
	case EStoryState::Island1_PuzzleSolved:
		return FText::FromString("Talk to the Listener.");
	case EStoryState::Island1_ReadyToLeave:
		return FText::FromString("Go back to the boat.");

	case EStoryState::Island2_TalkToNPCInside:
		return FText::FromString("Talk to the woman inside the house.");
	case EStoryState::Island2_FindGramophoneParts:
		return FText::FromString("Find what is needed to repair the gramophone.");
	case EStoryState::Island2_CraftMechanism:
		return FText::FromString("Craft the gramophone mechanism.");
	case EStoryState::Island2_InstallMechanism:
		return FText::FromString("Install the mechanism.");
	case EStoryState::Island2_PlayGramophone:
		return FText::FromString("Play the gramophone.");
	case EStoryState::Island2_ReturnToNPC:
		return FText::FromString("Talk to the woman again.");
	case EStoryState::Island2_ReadyToLeave:
		return FText::FromString("Go back to the boat.");

	case EStoryState::Island3_TalkToNPC:
		return FText::FromString("Talk to the Gatekeeper.");
	case EStoryState::Island3_FindPaper:
		return FText::FromString("Find paper.");
	case EStoryState::Island3_CraftNote:
		return FText::FromString("Draw the Gatekeeper.");
	case EStoryState::Island3_GiveNoteToNPC:
		return FText::FromString("Show the drawing to the Gatekeeper.");
	case EStoryState::Island3_EnterHouse:
		return FText::FromString("Enter the house.");
	case EStoryState::Island3_OpenPadlock:
		return FText::FromString("Open the padlock.");
	case EStoryState::Island3_OpenGate:
		return FText::FromString("Open the gate.");
	case EStoryState::Island3_ReadyToLeave:
		return FText::FromString("Go through the gate.");

	case EStoryState::Level2_ChooseIsland:
		return FText::FromString("Choose which island to explore.");
	case EStoryState::Level2_MotorIslandExplore:
	case EStoryState::Level2_MotorIslandFindParts:
		return FText::FromString("Find parts for the first engine half.");
	case EStoryState::Level2_MotorIslandCraftHalf:
		return FText::FromString("Craft the first engine half.");
	case EStoryState::Level2_MotorIslandSolved:
		return FText::FromString("Go back to the boat.");
	case EStoryState::Level2_LeverIslandExplore:
	case EStoryState::Level2_LeverIslandFindParts:
		return FText::FromString("Find parts for the second engine half.");
	case EStoryState::Level2_LeverIslandCraftHalf:
		return FText::FromString("Craft the second engine half.");
	case EStoryState::Level2_LeverIslandSolved:
		return FText::FromString("Go back to the boat.");
	case EStoryState::Level2_CompassBearerSpawned:
		return FText::FromString("Listen to the Compass Bearer.");
	case EStoryState::Level2_CompassReceived:
		return FText::FromString("Go to the lighthouse island.");

	case EStoryState::Lighthouse_NotReady:
		return FText::FromString("You are not ready for the lighthouse yet.");
	case EStoryState::Lighthouse_TalkToEntity:
		return FText::FromString("Talk to the Lost One.");
	case EStoryState::Lighthouse_EnterLighthouse:
		return FText::FromString("Enter the lighthouse.");
	case EStoryState::Lighthouse_Explore:
		return FText::FromString("Explore the lighthouse.");
	case EStoryState::Lighthouse_InstallEngine:
		return FText::FromString("Install the engine.");
	case EStoryState::Lighthouse_LightCutscene:
		return FText::FromString("Watch the lighthouse awaken.");
	case EStoryState::Lighthouse_AfterLightDialogue:
		return FText::FromString("Talk after the light returns.");
	case EStoryState::Lighthouse_EndingCutscene:
		return FText::FromString("See what happens next.");

	default:
		return FText::GetEmpty();
	}
}

bool AStoryFlowManager::AreAllMelodyPiecesFound(AProgressionManager* ProgressionManager) const
{
	if (!ProgressionManager)
	{
		return false;
	}

	int32 ValidFlagCount = 0;
	int32 FoundFlagCount = 0;

	for (const FName& Flag : MelodyPieceFlags)
	{
		if (Flag.IsNone())
		{
			continue;
		}

		ValidFlagCount++;

		if (ProgressionManager->HasFlag(Flag))
		{
			FoundFlagCount++;
		}
	}

	// If no melody piece flags were configured, do NOT count it as all found.
	if (ValidFlagCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AreAllMelodyPiecesFound: MelodyPieceFlags is empty or only contains None."));
		return false;
	}

	return FoundFlagCount == ValidFlagCount;
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
	TryAddItemToInventoryOnce(ProgressionManager, ShellItemID, ShellItemAddedToInventoryFlag, TEXT("Shell"));
}


void AStoryFlowManager::TryAddPenToInventory(AProgressionManager* ProgressionManager)
{
	TryAddItemToInventoryOnce(ProgressionManager, PenItemID, PenItemAddedToInventoryFlag, TEXT("Pen"));
}

void AStoryFlowManager::TryClearIsland1ItemsFromInventory(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager) return;

	// Kör bara en gång.
	if (ProgressionManager->HasFlag(Island1ItemsClearedFlag)) return;

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not clear Island 1 items: No player character."));
		return;
	}

	UInventoryComponent* InventoryComponent = PlayerCharacter->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not clear Island 1 items: No InventoryComponent."));
		return;
	}

	for (const FName& ItemID : Island1ItemsToClearOnLeave)
	{
		if (ItemID.IsNone()) continue;

		const bool bRemoved = InventoryComponent->RemoveItemByID(ItemID);
		UE_LOG(LogTemp, Warning, TEXT("Island 1 cleanup: %s -> %s"),
			*ItemID.ToString(), bRemoved ? TEXT("removed") : TEXT("not found"));
	}

	ProgressionManager->AddFlag(Island1ItemsClearedFlag);
}

void AStoryFlowManager::TryClearHomeItemsFromInventory(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager) return;

	if (ProgressionManager->HasFlag(HomeItemsClearedFlag)) return;

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not clear Home items: No player character."));
		return;
	}

	UInventoryComponent* InventoryComponent = PlayerCharacter->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not clear Home items: No InventoryComponent."));
		return;
	}

	for (const FName& ItemID : HomeItemsToClearOnLeave)
	{
		if (ItemID.IsNone()) continue;

		const bool bRemoved = InventoryComponent->RemoveItemByID(ItemID);
		UE_LOG(LogTemp, Warning, TEXT("Home cleanup: %s -> %s"),
			*ItemID.ToString(), bRemoved ? TEXT("removed") : TEXT("not found"));
	}

	ProgressionManager->AddFlag(HomeItemsClearedFlag);
}

void AStoryFlowManager::TryClearIsland2ItemsFromInventory(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager) return;

	if (ProgressionManager->HasFlag(Island2ItemsClearedFlag)) return;

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not clear Island 2 items: No player character."));
		return;
	}

	UInventoryComponent* InventoryComponent = PlayerCharacter->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not clear Island 2 items: No InventoryComponent."));
		return;
	}

	for (const FName& ItemID : Island2ItemsToClearOnLeave)
	{
		if (ItemID.IsNone()) continue;

		const bool bRemoved = InventoryComponent->RemoveItemByID(ItemID);
		UE_LOG(LogTemp, Warning, TEXT("Island 2 cleanup: %s -> %s"),
			*ItemID.ToString(), bRemoved ? TEXT("removed") : TEXT("not found"));
	}

	ProgressionManager->AddFlag(Island2ItemsClearedFlag);
}

void AStoryFlowManager::TryClearIsland3ItemsFromInventory(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager) return;

	if (ProgressionManager->HasFlag(Island3ItemsClearedFlag)) return;

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not clear Island 3 items: No player character."));
		return;
	}

	UInventoryComponent* InventoryComponent = PlayerCharacter->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not clear Island 3 items: No InventoryComponent."));
		return;
	}

	for (const FName& ItemID : Island3ItemsToClearOnLeave)
	{
		if (ItemID.IsNone()) continue;

		const bool bRemoved = InventoryComponent->RemoveItemByID(ItemID);
		UE_LOG(LogTemp, Warning, TEXT("Island 3 cleanup: %s -> %s"),
			*ItemID.ToString(), bRemoved ? TEXT("removed") : TEXT("not found"));
	}

	ProgressionManager->AddFlag(Island3ItemsClearedFlag);
}

void AStoryFlowManager::TryClearLighthouseItemsFromInventory(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager) return;

	if (ProgressionManager->HasFlag(LighthouseItemsClearedFlag)) return;

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not clear Lighthouse items: No player character."));
		return;
	}

	UInventoryComponent* InventoryComponent = PlayerCharacter->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not clear Lighthouse items: No InventoryComponent."));
		return;
	}

	for (const FName& ItemID : LighthouseItemsToClearOnInstall)
	{
		if (ItemID.IsNone()) continue;

		const bool bRemoved = InventoryComponent->RemoveItemByID(ItemID);
		UE_LOG(LogTemp, Warning, TEXT("Lighthouse cleanup: %s -> %s"),
			*ItemID.ToString(), bRemoved ? TEXT("removed") : TEXT("not found"));
	}

	ProgressionManager->AddFlag(LighthouseItemsClearedFlag);
}

void AStoryFlowManager::HandleItemPickedUp(FName ItemID, bool bFirstPickupEver)
{
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, ItemID]()
	{
		CacheRuntimeReferences();

		AProgressionManager* ProgressionManager = GetProgressionManager();
		if (!ProgressionManager)
		{
			return;
		}

		TryShowAllItemsFoundDialogue(ProgressionManager, ItemID);
		UpdateStoryFlow();
		UpdateObjectiveSystems(CurrentStoryState);
	}));
}

void AStoryFlowManager::TryShowAllItemsFoundDialogue(AProgressionManager* ProgressionManager, FName PickedUpItemID)
{
	if (!ProgressionManager)
	{
		return;
	}

	ADialogueManager* DialogueManager = GetDialogueManager();

	if (!DialogueManager)
	{
		return;
	}

	if (DialogueManager->IsDialogueActive())
	{
		return;
	}

	/*
	// Island 1: only check this when the picked up item was actually a melody piece.
	const bool bPickedUpMelodyPiece =
		MelodyPieceFlags.Contains(PickedUpItemID);

	if (bPickedUpMelodyPiece &&
		ProgressionManager->HasFlag(ArrivedIsland1Flag) &&
		AreAllMelodyPiecesFound(ProgressionManager) &&
		!ProgressionManager->HasFlag(Island1PuzzleSolvedFlag) &&
		!ProgressionManager->HasFlag(TalkedToListenerAfterPuzzleFlag) &&
		!ProgressionManager->HasFlag(Island1AllItemsDialogueShownFlag))
	{
		ProgressionManager->AddFlag(Island1AllItemsDialogueShownFlag);
		DialogueManager->ShowMessage(Island1AllItemsFoundMessage);
		return;
	}
	*/
	

	// Island 2
	if (ProgressionManager->HasFlag(ArrivedIsland2Flag) &&
		AreAllGramophonePartsFound(ProgressionManager) &&
		!ProgressionManager->HasFlag(GramophoneMechanismCraftedFlag) &&
		!ProgressionManager->HasFlag(Island2AllItemsDialogueShownFlag))
	{
		ProgressionManager->AddFlag(Island2AllItemsDialogueShownFlag);
		DialogueManager->ShowMessage(Island2AllItemsFoundMessage);
		return;
	}
	
	

	// Island 3
	if (ProgressionManager->HasFlag(ArrivedIsland3Flag) &&
		ProgressionManager->HasFlag(Island3PaperPickedUpFlag) &&
		ProgressionManager->HasFlag(PenItemAddedToInventoryFlag) &&
		!ProgressionManager->HasFlag(Island3NoteCraftedFlag) &&
		!ProgressionManager->HasFlag(Island3AllItemsDialogueShownFlag))
	{
		ProgressionManager->AddFlag(Island3AllItemsDialogueShownFlag);
		DialogueManager->ShowMessage(Island3AllItemsFoundMessage);
		return;
	}

	// Level 2 - motor island
	if (ProgressionManager->HasFlag(ArrivedLevel2MotorIslandFlag) &&
		ProgressionManager->HasFlag(CopperPipePickupFlag) &&
		ProgressionManager->HasFlag(HandWheelPipePickupFlag) &&
		!ProgressionManager->HasFlag(Level2MotorIslandSolvedFlag) &&
		!ProgressionManager->HasFlag(Level2MotorIslandAllItemsDialogueShownFlag))
	{
		ProgressionManager->AddFlag(Level2MotorIslandAllItemsDialogueShownFlag);
		DialogueManager->ShowMessage(Level2MotorIslandAllItemsFoundMessage);
		return;
	}

	// Level 2 - lever island
	if (ProgressionManager->HasFlag(ArrivedLevel2LeverIslandFlag) &&
		ProgressionManager->HasFlag(SparkPlugPickupFlag) &&
		ProgressionManager->HasFlag(IgnitionCapPickupFlag) &&
		!ProgressionManager->HasFlag(Level2LeverIslandSolvedFlag) &&
		!ProgressionManager->HasFlag(Level2LeverIslandAllItemsDialogueShownFlag))
	{
		ProgressionManager->AddFlag(Level2LeverIslandAllItemsDialogueShownFlag);
		DialogueManager->ShowMessage(Level2LeverIslandAllItemsFoundMessage);
		return;
	}
	
}

void AStoryFlowManager::TryScheduleMelodyOrderHint(AProgressionManager* ProgressionManager)
{
	if (!ProgressionManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Melody hint failed: No ProgressionManager."));
		return;
	}

	if (!ProgressionManager->HasFlag(TalkedToListenerAfterAllMelodyPiecesFlag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Melody hint not scheduled: TalkedToListenerAfterAllMelodyPieces flag is missing."));
		return;
	}

	if (ProgressionManager->HasFlag(Island1PuzzleSolvedFlag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Melody hint not scheduled: Puzzle already solved."));
		return;
	}

	if (ProgressionManager->HasFlag(MelodyOrderHintShownFlag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Melody hint not scheduled: Already shown."));
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(MelodyOrderHintTimerHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT("Melody hint not scheduled: Timer already active."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Melody hint scheduled. Lumi will speak in 3 seconds."));

	ProgressionManager->AddFlag(MelodyOrderHintShownFlag);

	GetWorldTimerManager().SetTimer(
		MelodyOrderHintTimerHandle,
		this,
		&AStoryFlowManager::ShowMelodyOrderHint,
		2.0f,
		false
	);
}

void AStoryFlowManager::ShowMelodyOrderHint()
{
	UE_LOG(LogTemp, Warning, TEXT("ShowMelodyOrderHint called."));

	AProgressionManager* ProgressionManager = GetProgressionManager();

	if (!ProgressionManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Melody hint failed: No ProgressionManager in ShowMelodyOrderHint."));
		return;
	}

	if (ProgressionManager->HasFlag(Island1PuzzleSolvedFlag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Melody hint cancelled: Puzzle already solved."));
		return;
	}

	ADialogueManager* DialogueManager = GetDialogueManager();

	if (!DialogueManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Melody hint failed: No DialogueManager found."));
		return;
	}

	if (DialogueManager->IsDialogueOrMessageVisible())
	{
		UE_LOG(LogTemp, Warning, TEXT("Melody hint waiting: Dialogue/message still visible. Retrying in 1 second."));

		GetWorldTimerManager().SetTimer(
			MelodyOrderHintTimerHandle,
			this,
			&AStoryFlowManager::ShowMelodyOrderHint,
			1.0f,
			false
		);

		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Melody hint shown."));

	DialogueManager->ShowMessage(MelodyOrderHintMessage);
}
