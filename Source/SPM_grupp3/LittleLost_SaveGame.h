#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ItemDataTypes.h"
#include "StoryFlowManager.h"
#include "LittleLost_SaveGame.generated.h"

/**
 * Single-slot gameplay save. Holds everything needed to restore a session.
 */
UCLASS()
class SPM_GRUPP3_API ULittleLost_SaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    /** Real-world timestamp this save was written. */
    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Meta")
    FDateTime SavedAtUtc;

    /** Level the player was in when saved. */
    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Player")
    FName CurrentLevelName;

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Player")
    FVector PlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Player")
    FRotator PlayerRotation = FRotator::ZeroRotator;

    /** Inventory contents. */
    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Inventory")
    TArray<FInventorySlot> InventorySlots;

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Inventory")
    int32 SelectedSlotIndex = 0;

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Inventory")
    bool bHasEverPickedUpItem = false;

    /** Progression flags from ProgressionManager. */
    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Progression")
    TSet<FName> ProgressFlags;

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Progression")
    FText CurrentObjectiveText;

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Progression")
    FName CurrentObjectiveID;

    /** Current StoryFlow state. */
    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Story")
    EStoryState CurrentStoryState = EStoryState::Home_Explore;
};
