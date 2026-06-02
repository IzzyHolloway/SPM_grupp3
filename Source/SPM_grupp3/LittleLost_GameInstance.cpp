
#include "LittleLost_GameInstance.h"
#include "LittleLost_SaveGame.h"
#include "InventoryComponent.h"
#include "ProgressionManager.h"
#include "CharacterAimi.h"
#include "BoatFunctionality.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

void ULittleLost_GameInstance::Init()
{
    Super::Init();
}

bool ULittleLost_GameInstance::HasSave() const
{
    return UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex);
}

void ULittleLost_GameInstance::NewGame(FName StartLevelName)
{
    PendingSave = nullptr;
    bShouldApplyOnNextWorldReady = false;

    if (HasSave())
    {
        UGameplayStatics::DeleteGameInSlot(SaveSlotName, UserIndex);
    }

    if (!StartLevelName.IsNone())
    {
        UGameplayStatics::OpenLevel(this, StartLevelName);
    }
}

// Loads the save file, opens the saved level, and flags that the world should be
// restored from PendingSave as soon as the new level is ready.
void ULittleLost_GameInstance::ContinueGame()
{
    if (!HasSave())
    {
        UE_LOG(LogTemp, Warning, TEXT("ContinueGame: no save exists."));
        return;
    }

    USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex);
    PendingSave = Cast<ULittleLost_SaveGame>(Loaded);
    if (!PendingSave)
    {
        UE_LOG(LogTemp, Warning, TEXT("ContinueGame: load returned null or wrong class."));
        return;
    }

    bShouldApplyOnNextWorldReady = true;

    const FName LevelToOpen = !PendingSave->CurrentLevelName.IsNone()
        ? PendingSave->CurrentLevelName
        : FName();

    if (!LevelToOpen.IsNone())
    {
        UGameplayStatics::OpenLevel(this, LevelToOpen);
    }
}

// In-game transition (e.g. sailing through the gate). Snapshots the current world into
// PendingSave -- which lives on the GameInstance and survives the level load -- then flags
// that the next level should restore from it. No disk write is needed for the transition;
// the snapshot is kept in memory.
void ULittleLost_GameInstance::TravelToLevel(FName LevelName, bool bSpawnInBoat)
{
    // Loop guard: Level 2 was duplicated from Level 1, so it contains the same BP_Gate.
    // When we arrive in Level 2 and ApplyToWorld restores the Island3PadlockSolved flag,
    // that gate's Tick sees the flag and calls TravelToLevel again -- which would reload the
    // same level forever. Refuse any request to travel to the level we are already in.
    const FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(this, true);
    if (!LevelName.IsNone() && LevelName.ToString() == CurrentLevel)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("TravelToLevel: ignoring request to travel to the current level '%s' (loop guard)."),
            *CurrentLevel);
        return;
    }

    CaptureFromWorld();
    bShouldApplyOnNextWorldReady = true;
    bForceSpawnInBoat = bSpawnInBoat;

    if (!LevelName.IsNone())
    {
        UGameplayStatics::OpenLevel(this, LevelName);
    }
}

// Capture the current world into PendingSave and write it to disk asynchronously
// so saving doesn't block the game thread.
void ULittleLost_GameInstance::SaveGameAsync()
{
    CaptureFromWorld();

    if (!PendingSave)
    {
        UE_LOG(LogTemp, Warning, TEXT("SaveGameAsync: nothing to save."));
        return;
    }

    FAsyncSaveGameToSlotDelegate Done;
    Done.BindUObject(this, &ULittleLost_GameInstance::OnAsyncSaveFinished);

    UGameplayStatics::AsyncSaveGameToSlot(PendingSave, SaveSlotName, UserIndex, Done);
}

void ULittleLost_GameInstance::OnAsyncSaveFinished(const FString& Slot, const int32 Index, bool bSuccess)
{
    UE_LOG(LogTemp, Log, TEXT("AsyncSave finished: slot=%s, success=%s"),
        *Slot, bSuccess ? TEXT("true") : TEXT("false"));
}

// Reads the current player, inventory and progression state into PendingSave
void ULittleLost_GameInstance::CaptureFromWorld()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (!PendingSave)
    {
        PendingSave = Cast<ULittleLost_SaveGame>(
            UGameplayStatics::CreateSaveGameObject(ULittleLost_SaveGame::StaticClass()));
        if (!PendingSave) return;
    }

    PendingSave->SavedAtUtc = FDateTime::UtcNow();
    // bRemovePrefix=true: strip the PIE prefix (e.g. "UEDPIE_0_Level1") so the saved name
    // matches the real level and ContinueGame's OpenLevel works in both editor and packaged builds.
    PendingSave->CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(this, true));

    // Find the player character. While riding the boat the controller possesses the BOAT
    // (a Pawn, not a Character), so GetPlayerCharacter() returns null -- we then look the
    // character up among the boat's attached actors.
    ACharacterAimi* Player = nullptr;
    PendingSave->bWasInBoat = false;

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        APawn* Pawn = PC->GetPawn();

        if (ABoatFunctionality* Boat = Cast<ABoatFunctionality>(Pawn))
        {
            // Player is currently riding the boat.
            PendingSave->bWasInBoat = true;
            PendingSave->BoatLocation = Boat->GetActorLocation();
            PendingSave->BoatRotation = Boat->GetActorRotation();

            TArray<AActor*> AttachedActors;
            Boat->GetAttachedActors(AttachedActors);
            for (AActor* Attached : AttachedActors)
            {
                if (ACharacterAimi* FoundPlayer = Cast<ACharacterAimi>(Attached))
                {
                    Player = FoundPlayer;
                    break;
                }
            }
        }
        else
        {
            Player = Cast<ACharacterAimi>(Pawn);
        }
    }

    // Player transform + inventory snapshot
    if (Player)
    {
        PendingSave->PlayerLocation = Player->GetActorLocation();
        PendingSave->PlayerRotation = Player->GetActorRotation();

        if (UInventoryComponent* Inv = Player->FindComponentByClass<UInventoryComponent>())
        {
            PendingSave->InventorySlots = Inv->InventorySlots;
            PendingSave->SelectedSlotIndex = Inv->SelectedSlotIndex;
            PendingSave->bHasEverPickedUpItem = Inv->bHasEverPickedUpItem;
        }
    }

    // Story / progression flags
    if (AProgressionManager* PM = Cast<AProgressionManager>(
        UGameplayStatics::GetActorOfClass(World, AProgressionManager::StaticClass())))
    {
        PendingSave->ProgressFlags = PM->GetAllFlags();
        PendingSave->CurrentObjectiveText = PM->GetCurrentObjectiveText();
        PendingSave->CurrentObjectiveID = PM->GetCurrentObjectiveID();
    }
}

// Restores world state from PendingSave. Call this after the loaded level is ready
void ULittleLost_GameInstance::ApplyToWorld()
{
    if (!bShouldApplyOnNextWorldReady || !PendingSave) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // The GameMode has already spawned & possessed the player character at a PlayerStart.
    ACharacterAimi* Player = Cast<ACharacterAimi>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player)
    {
        Player = Cast<ACharacterAimi>(
            UGameplayStatics::GetActorOfClass(World, ACharacterAimi::StaticClass()));
    }

    // Seat the player in the boat if they were riding it when the snapshot was taken,
    // OR if this transition explicitly asked for it (e.g. walking through the gate on land
    // but spawning into the next level already in the boat).
    const bool bBoardBoat = PendingSave->bWasInBoat || bForceSpawnInBoat;

    if (Player)
    {
        if (bBoardBoat)
        {
            // Re-seat the player in the boat (mirrors EnterBoat()).
            if (ABoatFunctionality* Boat = Cast<ABoatFunctionality>(
                    UGameplayStatics::GetActorOfClass(World, ABoatFunctionality::StaticClass())))
            {
                // Only restore the saved boat transform when the player genuinely was in the
                // boat. When forcing them in after a gate transition, leave the boat where the
                // destination level placed it.
                if (PendingSave->bWasInBoat)
                {
                    Boat->SetActorLocationAndRotation(PendingSave->BoatLocation, PendingSave->BoatRotation);
                }

                AController* PlayerController = Player->GetController(); // read before possession changes

                if (UCharacterMovementComponent* Move = Player->GetCharacterMovement())
                {
                    Move->DisableMovement();
                }

                Player->IsBoating = true;
                Player->AttachToActor(Boat,
                    FAttachmentTransformRules(EAttachmentRule::SnapToTarget,
                                              EAttachmentRule::SnapToTarget,
                                              EAttachmentRule::KeepRelative, true));
                Player->SetActorRelativeLocation(Boat->GetCharacterPositionOffset());

                if (PlayerController)
                {
                    PlayerController->Possess(Boat);
                }
            }
        }
        else
        {
            Player->SetActorLocationAndRotation(
                PendingSave->PlayerLocation,
                PendingSave->PlayerRotation);
        }

        // Inventory lives on the character and survives even while boating.
        if (UInventoryComponent* Inv = Player->FindComponentByClass<UInventoryComponent>())
        {
            Inv->InventorySlots = PendingSave->InventorySlots;
            Inv->SelectedSlotIndex = PendingSave->SelectedSlotIndex;
            Inv->bHasEverPickedUpItem = PendingSave->bHasEverPickedUpItem;
            Inv->OnInventoryUpdated.Broadcast();
        }
    }

    // Restore story flags + active objective
    if (AProgressionManager* PM = Cast<AProgressionManager>(
        UGameplayStatics::GetActorOfClass(World, AProgressionManager::StaticClass())))
    {
        for (const FName& Flag : PendingSave->ProgressFlags)
        {
            PM->AddFlag(Flag);
        }
        PM->SetCurrentObjectiveText(PendingSave->CurrentObjectiveText);
        PM->SetCurrentObjectiveID(PendingSave->CurrentObjectiveID);
    }

    // Only apply once per load
    bShouldApplyOnNextWorldReady = false;
    bForceSpawnInBoat = false;
}
