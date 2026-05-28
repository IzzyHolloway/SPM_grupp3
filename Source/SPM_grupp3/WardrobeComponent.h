#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WardrobeDataTypes.h"
#include "WardrobeComponent.generated.h"

class AWardrobe;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWardrobeUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquippedCoatChanged, FName, NewCoatID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCoatUnlocked, FName, CoatID, bool, bFirstCoatEver);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPM_GRUPP3_API UWardrobeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWardrobeComponent();

protected:
    virtual void BeginPlay() override;

public:
    // Events the UI / character visuals bind to

    UPROPERTY(BlueprintAssignable, Category = "Wardrobe|Events")
    FOnWardrobeUpdated OnWardrobeUpdated;            // Slots / selection / open state changed.

    UPROPERTY(BlueprintAssignable, Category = "Wardrobe|Events")
    FOnEquippedCoatChanged OnEquippedCoatChanged;    // The coat Lumi wears changed.

    UPROPERTY(BlueprintAssignable, Category = "Wardrobe|Events")
    FOnCoatUnlocked OnCoatUnlocked;                  // A new coat pickup was collected.

    // Configuration

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wardrobe")
    TObjectPtr<UDataTable> CoatDataTable;            // All coats (rows of FCoatDetail). BeginPlay seeds slots from this.

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wardrobe|Layout")
    int32 GridColumns = 2;                           // Columns used for grid based stick navigation.

    // State

    UPROPERTY(BlueprintReadOnly, Category = "Wardrobe|State")
    TArray<FWardrobeSlot> Slots;                     // One entry per coat in the data table, fixed after BeginPlay.

    UPROPERTY(BlueprintReadOnly, Category = "Wardrobe|State")
    FName EquippedCoatID = NAME_None;                // The coat Lumi currently wears.

    UPROPERTY(BlueprintReadOnly, Category = "Wardrobe|Selection")
    int32 SelectedSlotIndex = 0;                     // Slot currently highlighted in the UI (hover/cursor).

    // The AWardrobe actor that is currently driving the UI, or null if closed.
    // Set by AWardrobe::OpenWardrobe, cleared by CloseWardrobe.
    // Lets external systems (e.g. character input) call ActiveWardrobeActor->CloseWardrobe().
    UPROPERTY(BlueprintReadOnly, Category = "Wardrobe|State")
    TWeakObjectPtr<AWardrobe> ActiveWardrobeActor;

    // API

    UFUNCTION(BlueprintPure, Category = "Wardrobe|State")
    bool IsWardrobeOpen() const { return bIsWardrobeOpen; }

    UFUNCTION(BlueprintCallable, Category = "Wardrobe|State")
    void SetWardrobeOpen(bool bOpen);

    // Unlock a coat. Called by ACoatPickup. Returns false if the coat is already unlocked or unknown.
    UFUNCTION(BlueprintCallable, Category = "Wardrobe")
    bool UnlockCoat(FName CoatID);

    // Equip the coat at the currently selected slot. No-op if the slot is locked or already equipped.
    UFUNCTION(BlueprintCallable, Category = "Wardrobe|Input")
    void EquipSelected();

    // Directly equip a coat by ID (useful for the BeginPlay default).
    UFUNCTION(BlueprintCallable, Category = "Wardrobe")
    bool EquipCoat(FName CoatID);

    // Move the cursor in a 2D grid, skipping locked slots.
    UFUNCTION(BlueprintCallable, Category = "Wardrobe|Input")
    void MoveSelectionGrid(int32 DeltaX, int32 DeltaY);

    UFUNCTION(BlueprintCallable, Category = "Wardrobe|Input")
    void MoveSelection(int32 Direction);

    // Set the cursor to a specific slot. Used by mouse hover.
    UFUNCTION(BlueprintCallable, Category = "Wardrobe|Input")
    void SetSelectedSlot(int32 NewIndex);

    // Snap the cursor to the currently equipped coat, if any (used when opening the wardrobe).
    UFUNCTION(BlueprintCallable, Category = "Wardrobe|Selection")
    void SelectEquippedSlot();

    UFUNCTION(BlueprintPure, Category = "Wardrobe")
    bool IsSlotUnlocked(int32 SlotIndex) const;

    UFUNCTION(BlueprintPure, Category = "Wardrobe")
    bool IsSlotEquipped(int32 SlotIndex) const;

    UFUNCTION(BlueprintPure, Category = "Wardrobe")
    bool IsSlotSelected(int32 SlotIndex) const { return SlotIndex == SelectedSlotIndex; }

    UFUNCTION(BlueprintPure, Category = "Wardrobe")
    bool IsCoatUnlocked(FName CoatID) const;

    UFUNCTION(BlueprintPure, Category = "Wardrobe")
    int32 FindSlotIndexForCoat(FName CoatID) const;

private:
    UPROPERTY(BlueprintReadOnly, Category = "Wardrobe|State",
              meta=(AllowPrivateAccess="true"))
    bool bIsWardrobeOpen = false;

    UPROPERTY(BlueprintReadOnly, Category = "Wardrobe|State",
              meta=(AllowPrivateAccess="true"))
    bool bHasEverUnlockedCoat = false;
};
