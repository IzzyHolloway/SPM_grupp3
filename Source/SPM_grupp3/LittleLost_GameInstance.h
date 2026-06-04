#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "LittleLost_GameInstance.generated.h"

class ULittleLost_SaveGame;

UCLASS()
class SPM_GRUPP3_API ULittleLost_GameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    // Save slot configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SaveGame")
    FString SaveSlotName = TEXT("MainSave");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SaveGame")
    int32 UserIndex = 0;

    // API used by the Main Menu and gameplay code.

    UFUNCTION(BlueprintPure, Category = "SaveGame")
    bool HasSave() const;                       // True if a save file exists on disk.

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void NewGame(FName StartLevelName);         // Wipe any old save and open the start level.

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void ContinueGame();                        // Load the saved file and open its level.

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void TravelToLevel(FName LevelName, bool bSpawnInBoat = false);  // In-game level transition that preserves player/inventory/flags. If bSpawnInBoat is true, the player is seated in the destination level's boat even if they were on land when leaving.

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void SaveGameAsync();                       // Capture world state and write to disk in the background.

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void ApplyToWorld();                        // Apply the loaded save to the current world (call after the level finishes loading).

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void CaptureFromWorld();                    // Read the current world state into PendingSave.

    // Store a safe checkpoint/respawn location for the player.
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Respawn")
    void SetLastSavedPlayerTransform(FVector Location, FRotator Rotation);

    // Convenience helper for checkpoints: stores the current player transform as the safe respawn point.
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Respawn")
    void SaveCurrentPlayerTransformAsLastSaved();

    // Move the current player back to LastSavedLocation. Returns false if no safe location exists yet.
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Respawn")
    bool RespawnPlayerAtLastSavedLocation();

    // True if the carried-over save already has this coat unlocked. Used by coat pickups in a
    // duplicated level (e.g. Level 2) to remove themselves if the player already collected them.
    UFUNCTION(BlueprintPure, Category = "SaveGame")
    bool IsCoatUnlockedInSave(FName CoatID) const;

    // --- Audio settings -------------------------------------------------------
    // Master volume (0..1) used by the sound menu. Lives on the GameInstance so it
    // survives level loads and is available even in the main menu (where there is no
    // player character). Replaces the old per-character VolumeValue the WBP_Volume used.
    UPROPERTY(BlueprintReadWrite, Category = "Settings")
    float MasterVolume = 1.0f;

    // Clamp to 0..1, store, and apply immediately.
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetMasterVolume(float NewVolume);

    // Push the current MasterVolume to the FMOD master bus + the engine sound mix.
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ApplyMasterVolume();

protected:
    virtual void Init() override;

private:
    // Loaded or in-progress save kept in memory until written / applied.
    UPROPERTY(Transient)
    TObjectPtr<ULittleLost_SaveGame> PendingSave;

    // Flag for ContinueGame -> ApplyToWorld: true while waiting for the new level to be ready.
    bool bShouldApplyOnNextWorldReady = false;

    // Set by TravelToLevel(bSpawnInBoat=true): seat the player in the destination level's
    // boat (at the boat's level-placed position) even though they were on land when leaving.
    bool bForceSpawnInBoat = false;

    ULittleLost_SaveGame* GetOrCreatePendingSave();
    bool HasUsableSavedLocation() const;

    void OnAsyncSaveFinished(const FString& Slot, const int32 Index, bool bSuccess);
};
