
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ItemDataTypes.h"
#include "StoryFlowManager.h"
#include "LittleLost_SaveGame.generated.h"


UCLASS()
class SPM_GRUPP3_API ULittleLost_SaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // Meta
    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Meta")
    FDateTime SavedAtUtc;                   // When the save was created.

    // Player
    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Player")
    FName CurrentLevelName;                 // Level the player was in.

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Player")
    FVector PlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Player")
    FRotator PlayerRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Player")
    bool bWasInBoat = false;                // True if the player was riding the boat when saved.

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Player")
    FVector BoatLocation = FVector::ZeroVector;   // Boat transform, used to respawn the player in the boat.

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Player")
    FRotator BoatRotation = FRotator::ZeroRotator;

    // Inventory
    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Inventory")
    TArray<FInventorySlot> InventorySlots;  // Snapshot of the inventory slots.

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Inventory")
    int32 SelectedSlotIndex = 0;

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Inventory")
    bool bHasEverPickedUpItem = false;      // Used for one-time tutorials.

    // Progression
    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Progression")
    TSet<FName> ProgressFlags;              // All story flags set so far.

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Progression")
    FText CurrentObjectiveText;             // Current objective shown to the player.

    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Progression")
    FName CurrentObjectiveID;

    // Story
    UPROPERTY(BlueprintReadWrite, Category = "SaveGame|Story")
    EStoryState CurrentStoryState = EStoryState::Home_Explore;
};
