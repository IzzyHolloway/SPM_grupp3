#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemDataTypes.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPM_GRUPP3_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnInventoryUpdated OnInventoryUpdated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FInventorySlot> InventorySlots;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Layout")
    int32 SlotCount = 6;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Layout")
    int32 GridColumns = 2;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|Selection")
    int32 SelectedSlotIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
    TObjectPtr<UDataTable> RecipeDataTable;

    UFUNCTION(BlueprintPure, Category = "Inventory|State")
    bool IsWorkbenchOpen() const { return bIsWorkbenchOpen; }

    /** Owned by ACraftingStation; do not set from gameplay code. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|State")
    void SetWorkbenchOpen(bool bOpen);

    /** Use from the character's inventory-toggle input handler. */
    UFUNCTION(BlueprintPure, Category = "Inventory|State")
    bool CanToggleInventoryUI() const { return !bIsWorkbenchOpen; }

    UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
    void MoveSelection(int32 Direction);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
    void MoveSelectionGrid(int32 DeltaX, int32 DeltaY);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
    void SetSelectedSlot(int32 NewIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Selection")
    void SelectFirstAvailableSlot();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
    void ToggleItemOnWorkbench();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void CraftItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearWorkbench();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItemToInventory(FName ItemToAdd, int32 Quantity);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsSlotOccupied(int32 SlotIndex) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsSlotSelected(int32 SlotIndex) const { return SlotIndex == SelectedSlotIndex; }

private:
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State",
              meta=(AllowPrivateAccess="true"))
    bool bIsWorkbenchOpen = false;
};
