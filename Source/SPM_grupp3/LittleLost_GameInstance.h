#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "LittleLost_GameInstance.generated.h"

class ULittleLost_SaveGame;

/**
 * Owns save/load. Persists across level changes; single-slot for now.
 *
 * Usage from BP:
 *   - MainMenu's "New Game"  -> NewGame(StartLevelName)
 *   - MainMenu's "Continue"  -> ContinueGame()  (button greyed out if HasSave() == false)
 *   - PauseMenu's "Save"     -> SaveGameAsync()
 *   - After a level loads    -> call ApplyToWorld()  (from your character's BeginPlay,
 *                                                     once Inventory/ProgressionManager exist)
 */
UCLASS()
class SPM_GRUPP3_API ULittleLost_GameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    /** Slot name on disk. Single slot for now. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SaveGame")
    FString SaveSlotName = TEXT("MainSave");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SaveGame")
    int32 UserIndex = 0;

    /** Returns true if a save file exists on disk. Used to enable/disable Continue. */
    UFUNCTION(BlueprintPure, Category = "SaveGame")
    bool HasSave() const;

    /** Wipes in-memory data and opens StartLevel. Use for New Game. */
    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void NewGame(FName StartLevelName);

    /** Loads save from disk, then opens the saved level. ApplyToWorld() runs on next world load. */
    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void ContinueGame();

    /** Captures current world state and writes async to disk. Returns immediately. */
    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void SaveGameAsync();

    /**
     * Call from the player's BeginPlay (or similar) after a level loads.
     * Pushes saved data into Inventory / ProgressionManager / Player.
     * Safe to call when there is no pending data — does nothing then.
     */
    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void ApplyToWorld();

    /** Reads current world state into PendingSave. Mainly for internal use, exposed for testing. */
    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void CaptureFromWorld();

protected:
    virtual void Init() override;

private:
    /** In-memory save data. Held across level changes. */
    UPROPERTY(Transient)
    TObjectPtr<ULittleLost_SaveGame> PendingSave;

    /** True when ContinueGame loaded data and we are waiting for the world to be ready to apply it. */
    bool bShouldApplyOnNextWorldReady = false;

    void OnAsyncSaveFinished(const FString& Slot, const int32 Index, bool bSuccess);
};
