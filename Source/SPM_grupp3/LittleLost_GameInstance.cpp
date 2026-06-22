
#include "LittleLost_GameInstance.h"
#include "LittleLost_SaveGame.h"
#include "InventoryComponent.h"
#include "WardrobeComponent.h"
#include "ProgressionManager.h"
#include "StoryFlowManager.h"
#include "CharacterAimi.h"
#include "BoatFunctionality.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "FMODBlueprintStatics.h"
#include "FMODBus.h"
#include "FMODStudioModule.h"
#include "FMOD/fmod_studio.hpp"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"

void ULittleLost_GameInstance::Init()
{
    Super::Init();

    // Re-apply the saved master volume after every level load (see OnPostLoadMap).
    PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
        this, &ULittleLost_GameInstance::OnPostLoadMap);
}

void ULittleLost_GameInstance::Shutdown()
{
    if (PostLoadMapHandle.IsValid())
    {
        FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
        PostLoadMapHandle.Reset();
    }

    Super::Shutdown();
}


void ULittleLost_GameInstance::OnPostLoadMap(UWorld* )
{
    ApplyMasterVolume();
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

ULittleLost_SaveGame* ULittleLost_GameInstance::GetOrCreatePendingSave()
{
    if (!PendingSave)
    {
        PendingSave = Cast<ULittleLost_SaveGame>(
            UGameplayStatics::CreateSaveGameObject(ULittleLost_SaveGame::StaticClass()));
    }

    return PendingSave;
}

bool ULittleLost_GameInstance::HasUsableSavedLocation() const
{
    return PendingSave
        && PendingSave->bHasLastSavedLocation
        && !PendingSave->LastSavedLocation.IsNearlyZero();
}

void ULittleLost_GameInstance::SetLastSavedPlayerTransform(FVector Location, FRotator Rotation)
{
    if (Location.IsNearlyZero())
    {
        UE_LOG(LogTemp, Warning, TEXT("SetLastSavedPlayerTransform ignored zero location."));
        return;
    }

    ULittleLost_SaveGame* Save = GetOrCreatePendingSave();
    if (!Save)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetLastSavedPlayerTransform failed: could not create save object."));
        return;
    }

    Save->LastSavedLocation = Location;
    Save->LastSavedRotation = Rotation;
    Save->bHasLastSavedLocation = true;
}

void ULittleLost_GameInstance::SaveCurrentPlayerTransformAsLastSaved()
{
    ACharacterAimi* Player = Cast<ACharacterAimi>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player)
    {
        Player = Cast<ACharacterAimi>(
            UGameplayStatics::GetActorOfClass(GetWorld(), ACharacterAimi::StaticClass()));
    }

    if (!Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("SaveCurrentPlayerTransformAsLastSaved failed: no player found."));
        return;
    }

    SetLastSavedPlayerTransform(Player->GetActorLocation(), Player->GetActorRotation());
}

bool ULittleLost_GameInstance::RespawnPlayerAtLastSavedLocation()
{
    if (!HasUsableSavedLocation())
    {
        UE_LOG(LogTemp, Warning, TEXT("RespawnPlayerAtLastSavedLocation failed: no LastSavedLocation has been set."));
        return false;
    }

    ACharacterAimi* Player = Cast<ACharacterAimi>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player)
    {
        Player = Cast<ACharacterAimi>(
            UGameplayStatics::GetActorOfClass(GetWorld(), ACharacterAimi::StaticClass()));
    }

    if (!Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("RespawnPlayerAtLastSavedLocation failed: no player found."));
        return false;
    }

    Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Player->IsBoating = false;

    if (UCharacterMovementComponent* Move = Player->GetCharacterMovement())
    {
        Move->StopMovementImmediately();
        Move->SetMovementMode(MOVE_Walking);
    }

    Player->SetActorLocationAndRotation(
        PendingSave->LastSavedLocation,
        PendingSave->LastSavedRotation,
        false,
        nullptr,
        ETeleportType::TeleportPhysics);

    return true;
}

// Reads the current player, inventory and progression state into PendingSave
void ULittleLost_GameInstance::CaptureFromWorld()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (!GetOrCreatePendingSave()) return;

    PendingSave->SavedAtUtc = FDateTime::UtcNow();
    // bRemovePrefix=true: strip the PIE prefix (e.g. "UEDPIE_0_Level1") so the saved name
    // matches the real level and ContinueGame's OpenLevel works in both editor and packaged builds.
    PendingSave->CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(this, true));

    if (ABoatFunctionality* WorldBoat = Cast<ABoatFunctionality>(
        UGameplayStatics::GetActorOfClass(World, ABoatFunctionality::StaticClass())))
    {
        PendingSave->BoatLocation = WorldBoat->GetActorLocation();
        PendingSave->BoatRotation = WorldBoat->GetActorRotation();
        PendingSave->bHasBoatTransform = true;
    }

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
            PendingSave->bHasBoatTransform = true;

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

        if (!PendingSave->bHasLastSavedLocation && !PendingSave->PlayerLocation.IsNearlyZero())
        {
            PendingSave->LastSavedLocation = PendingSave->PlayerLocation;
            PendingSave->LastSavedRotation = PendingSave->PlayerRotation;
            PendingSave->bHasLastSavedLocation = true;
        }

        if (UInventoryComponent* Inv = Player->FindComponentByClass<UInventoryComponent>())
        {
            PendingSave->InventorySlots = Inv->InventorySlots;
            PendingSave->SelectedSlotIndex = Inv->SelectedSlotIndex;
            PendingSave->bHasEverPickedUpItem = Inv->bHasEverPickedUpItem;
        }

        // Wardrobe: which coats are unlocked + the one Lumi is wearing.
        if (UWardrobeComponent* Wardrobe = Player->FindComponentByClass<UWardrobeComponent>())
        {
            PendingSave->WardrobeSlots = Wardrobe->Slots;
            PendingSave->EquippedCoatID = Wardrobe->EquippedCoatID;
            PendingSave->bHasEverUnlockedCoat = Wardrobe->HasEverUnlockedCoat();
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

// True if the in-memory save (which survives the level load) already has this coat unlocked.
// Returns false when there is no save yet (e.g. a fresh Level 1 playthrough) so every pickup
// stays put. In Level 2 (a duplicate of Level 1) the save carries the wardrobe over, so coats
// the player already found report true and their pickups remove themselves.
bool ULittleLost_GameInstance::IsCoatUnlockedInSave(FName CoatID) const
{
    if (!PendingSave || CoatID.IsNone())
    {
        return false;
    }

    for (const FWardrobeSlot& Slot : PendingSave->WardrobeSlots)
    {
        if (Slot.CoatID == CoatID && Slot.bUnlocked)
        {
            return true;
        }
    }
    return false;
}

// Restores world state from PendingSave. Call this after the loaded level is ready
void ULittleLost_GameInstance::ApplyToWorld()
{
    if (!bShouldApplyOnNextWorldReady || !PendingSave) return;

    UWorld* World = GetWorld();
    if (!World) return;

    AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
        UGameplayStatics::GetActorOfClass(World, AProgressionManager::StaticClass()));
    if (!ProgressionManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyToWorld: ProgressionManager not ready yet. Retrying next tick."));
        World->GetTimerManager().SetTimerForNextTick(this, &ULittleLost_GameInstance::ApplyToWorld);
        return;
    }

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
    ABoatFunctionality* Boat = Cast<ABoatFunctionality>(
        UGameplayStatics::GetActorOfClass(World, ABoatFunctionality::StaticClass()));

    // ContinueGame should restore the boat to the island/dock where it was saved, even when
    // Lumi saved on land. Gate transitions that force the player into a boat should keep the
    // destination level's placed boat transform instead.
    if (Boat && PendingSave->bHasBoatTransform && !bForceSpawnInBoat)
    {
        Boat->SetActorLocationAndRotation(PendingSave->BoatLocation, PendingSave->BoatRotation);
    }

    if (Player)
    {
        if (bBoardBoat)
        {
            // Re-seat the player in the boat (mirrors EnterBoat()).
            if (Boat)
            {
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
            if (!PendingSave->PlayerLocation.IsNearlyZero())
            {
                Player->SetActorLocationAndRotation(
                    PendingSave->PlayerLocation,
                    PendingSave->PlayerRotation);
            }
            else if (HasUsableSavedLocation())
            {
                Player->SetActorLocationAndRotation(
                    PendingSave->LastSavedLocation,
                    PendingSave->LastSavedRotation);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("ApplyToWorld: saved player location is zero and no LastSavedLocation exists. Keeping PlayerStart location."));
            }
        }

        // Inventory lives on the character and survives even while boating.
        if (UInventoryComponent* Inv = Player->FindComponentByClass<UInventoryComponent>())
        {
            Inv->InventorySlots = PendingSave->InventorySlots;
            Inv->SelectedSlotIndex = PendingSave->SelectedSlotIndex;
            Inv->bHasEverPickedUpItem = PendingSave->bHasEverPickedUpItem;
            Inv->OnInventoryUpdated.Broadcast();
        }

        // Wardrobe: restore unlocked coats + re-equip the saved coat. Only overwrite the slot
        // list when the save actually has one (avoids wiping a fresh game's defaults). Broadcasting
        // OnEquippedCoatChanged makes the character visuals + preview update to the carried coat.
        if (UWardrobeComponent* Wardrobe = Player->FindComponentByClass<UWardrobeComponent>())
        {
            if (PendingSave->WardrobeSlots.Num() > 0)
            {
                Wardrobe->Slots = PendingSave->WardrobeSlots;
            }
            if (!PendingSave->EquippedCoatID.IsNone())
            {
                Wardrobe->EquippedCoatID = PendingSave->EquippedCoatID;
            }
            // Restore the "has ever unlocked a coat" flag so the first-coat tutorial doesn't
            // re-trigger when the player picks up a coat in the duplicated level.
            Wardrobe->SetHasEverUnlockedCoat(PendingSave->bHasEverUnlockedCoat);

            Wardrobe->OnWardrobeUpdated.Broadcast();
            if (!Wardrobe->EquippedCoatID.IsNone())
            {
                // Broadcast now for listeners already bound...
                Wardrobe->OnEquippedCoatChanged.Broadcast(Wardrobe->EquippedCoatID);

                // ...and again next tick. The character's coat material is swapped by a Blueprint
                // that binds to OnEquippedCoatChanged in its own BeginPlay, which may run AFTER this
                // restore. Re-broadcasting next tick (once every actor's BeginPlay has run) guarantees
                // the material actually applies after a level transition.
                TWeakObjectPtr<UWardrobeComponent> WeakWardrobe = Wardrobe;
                World->GetTimerManager().SetTimerForNextTick([WeakWardrobe]()
                {
                    if (WeakWardrobe.IsValid())
                    {
                        WeakWardrobe->BroadcastEquippedCoat();
                    }
                });
            }
        }
    }

    // Restore story flags + active objective
    for (const FName& Flag : PendingSave->ProgressFlags)
    {
        ProgressionManager->AddFlag(Flag);
    }
    ProgressionManager->SetCurrentObjectiveText(PendingSave->CurrentObjectiveText);
    ProgressionManager->SetCurrentObjectiveID(PendingSave->CurrentObjectiveID);

    if (AStoryFlowManager* StoryFlowManager = Cast<AStoryFlowManager>(
        UGameplayStatics::GetActorOfClass(World, AStoryFlowManager::StaticClass())))
    {
        StoryFlowManager->RefreshFromProgression();
    }
    
    // Aimi la till denna
    // After a level transition, force the controller back into a clean gameplay input state.
    // This prevents later UI (like crafting/inventory) from inheriting stale focus or input mode.
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
        PC->bShowMouseCursor = false;
    }

    // Only apply once per load
    bShouldApplyOnNextWorldReady = false;
    bForceSpawnInBoat = false;
}

void ULittleLost_GameInstance::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    ApplyMasterVolume();
}

// Applies MasterVolume to BOTH audio systems this game uses, so it controls everything
// regardless of which engine a given sound goes through:
//
//   1) FMOD (footsteps, water, boat, gate, ...): the master bus "bus:/" -- every FMOD event
//      ultimately routes through it. We target the master bus via the studio system instead of a
//      named sub-bus asset (e.g. fmod_ljud): BusSetVolume(named bus) only affects events routed
//      through that exact bus, and silently does nothing if its cached GUID no longer matches the
//      FMOD project after a rebuild.
//
//   2) Native UE audio (music, pickups, dialogue, ...): a SoundMix override per top-level sound
//      class. This project's sound classes are NOT all parented under one master class, so the
//      old code (which overrode only MasterSFX) reached the menu music -- SC_mainMenu is a child
//      of MasterSFX -- but never the in-level music on SC_BackgroundMusic, which is its own root
//      class. We override each root we use; bApplyToChildren=true cascades to their child classes.
void ULittleLost_GameInstance::ApplyMasterVolume()
{
    // MUSIC-ONLY volume. The slider lowers only the music sound classes; SFX, footsteps, UI,
    // dialogue, FMOD audio and cutscene sound effects are deliberately left at full volume.
    // bApplyToChildren=true, so any child sound class of these (e.g. a cutscene-music class
    // parented under SC_BackgroundMusic) follows along too. For a cutscene's music to obey this
    // slider, its Sound Wave/Cue must use one of these classes (or a child of them).
    USoundMix* Mix = LoadObject<USoundMix>(nullptr, TEXT("/Game/Audio/SCM_MasterVolume.SCM_MasterVolume"));
    USoundClass* MusicClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/Audio/music/Background/SC_BackgroundMusic.SC_BackgroundMusic"));

    // Diagnostic: shows on every slider change whether the mix + music class actually loaded,
    // and the value we apply. If both say OK but the music doesn't get quieter, the playing
    // music is routing through a DIFFERENT sound class (e.g. the menu-music cue is tagged MasterSFX).
    UE_LOG(LogTemp, Warning, TEXT("ApplyMasterVolume: vol=%.2f  Mix=%s  MusicClass(SC_BackgroundMusic)=%s"),
        MasterVolume,
        Mix ? TEXT("OK") : TEXT("NULL"),
        MusicClass ? TEXT("OK") : TEXT("NULL"));

    if (Mix && MusicClass)
    {
        UGameplayStatics::SetSoundMixClassOverride(this, Mix, MusicClass, MasterVolume, 1.0f, 1.0f, true);
        UGameplayStatics::PushSoundMixModifier(this, Mix);
    }
}
