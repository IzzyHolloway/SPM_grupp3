#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemDataTypes.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UInventoryComponent : public UActorComponent
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

    /** How many columns the inventory grid has. 6 slots / 2 cols = 3 rows. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Layout")
    int32 GridColumns = 2;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|Selection")
    int32 SelectedSlotIndex;

    /** True while the workbench UI is open. Other UI (regular inventory) should respect this. */
    UPROPERTY(BlueprintReadWrite, Category = "Inventory|State")
    bool bIsWorkbenchOpen = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
    class UDataTable* RecipeDataTable;

    // ----- Navigation -----
    /** Legacy 1D move. Kept for back-compat but prefer MoveSelectionGrid. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
    void MoveSelection(int32 Direction);

    /** Move on the 2D grid. DeltaX = ±1 horizontal, DeltaY = ±1 vertical. Skips empty slots. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
    void MoveSelectionGrid(int32 DeltaX, int32 DeltaY);

    /** Set selection directly. Used by mouse clicks. Ignored if the target slot is empty. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
    void SetSelectedSlot(int32 NewIndex);

    /** Snap the selector to the first slot that has an item. Call this when the workbench opens. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Selection")
    void SelectFirstAvailableSlot();

    // ----- Crafting / Workbench -----
    UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
    void ToggleItemOnWorkbench();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void CraftItem();

    /** Remove all items from the workbench surface (does NOT delete them — they stay in inventory). */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearWorkbench();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItemToInventory(FName ItemToAdd, int32 Quantity);

    // ----- Queries (handy for widget bindings) -----
    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsSlotOccupied(int32 SlotIndex) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsSlotSelected(int32 SlotIndex) const { return SlotIndex == SelectedSlotIndex; }
};