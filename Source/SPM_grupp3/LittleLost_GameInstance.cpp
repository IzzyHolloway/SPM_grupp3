#include "LittleLost_GameInstance.h"
#include "LittleLost_SaveGame.h"
#include "InventoryComponent.h"
#include "ProgressionManager.h"
#include "StoryFlowManager.h"
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
    // Wipe in-memory data so ApplyToWorld() in the new level becomes a no-op.
    PendingSave = nullptr;
    bShouldApplyOnNextWorldReady = false;

    // Optionally also delete the on-disk save so HasSave() returns false until next Save.
    // Comment this out if you want New Game to coexist with an existing save until first Save.
    if (HasSave())
    {
        UGameplayStatics::DeleteGameInSlot(SaveSlotName, UserIndex);
    }

    if (!StartLevelName.IsNone())
    {
        UGameplayStatics::OpenLevel(this, StartLevelName);
    }
}

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
    // If no level was stored, caller can OpenLevel themselves; ApplyToWorld will still run.
}

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

    // Player position/rotation + inventory.
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

    // Progression flags + current objective.
    if (AProgressionManager* PM = Cast<AProgressionManager>(
        UGameplayStatics::GetActorOfClass(World, AProgressionManager::StaticClass())))
    {
        PendingSave->ProgressFlags = PM->GetAllFlags();
        PendingSave->CurrentObjectiveText = PM->GetCurrentObjectiveText();
        PendingSave->CurrentObjectiveID = PM->GetCurrentObjectiveID();
    }

    // CurrentStoryState is recomputed each tick from progression flags by StoryFlowManager,
    // so we don't need to save/restore it explicitly. Restoring the flags is enough.
}

void ULittleLost_GameInstance::ApplyToWorld()
{
    if (!bShouldApplyOnNextWorldReady || !PendingSave) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Restore player position + inventory.
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

    // Restore progression flags (re-add each via public API).
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

    bShouldApplyOnNextWorldReady = false;
}
