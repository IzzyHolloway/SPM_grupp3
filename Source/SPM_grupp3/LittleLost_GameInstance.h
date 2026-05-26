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
    void SaveGameAsync();                       // Capture world state and write to disk in the background.

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void ApplyToWorld();                        // Apply the loaded save to the current world (call after the level finishes loading).

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void CaptureFromWorld();                    // Read the current world state into PendingSave.

protected:
    virtual void Init() override;

private:
    // Loaded or in-progress save kept in memory until written / applied.
    UPROPERTY(Transient)
    TObjectPtr<ULittleLost_SaveGame> PendingSave;

    // Flag for ContinueGame -> ApplyToWorld: true while waiting for the new level to be ready.
    bool bShouldApplyOnNextWorldReady = false;

    void OnAsyncSaveFinished(const FString& Slot, const int32 Index, bool bSuccess);
};
