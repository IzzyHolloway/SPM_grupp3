
#include "LittleLost_GameInstance.h"
#include "LittleLost_SaveGame.h"
#include "InventoryComponent.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

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
    PendingSave->CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(this));

    // Player transform + inventory snapshot
    if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0))
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

    // Restore player transform + inventory
    if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0))
    {
        Player->SetActorLocationAndRotation(
            PendingSave->PlayerLocation,
            PendingSave->PlayerRotation);

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
}
