
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemDataTypes.h"
#include "InventoryComponent.generated.h"

class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCraftSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemPickedUp, FName, ItemID, bool, bFirstPickupEver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemCrafted, FName, ItemID);

// Fired when a successful craft should open a puzzle widget instead of granting the item directly.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPuzzleCraftRequested, FName, ResultItemID, TSubclassOf<UUserWidget>, PuzzleWidgetClass);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPM_GRUPP3_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

protected:
    virtual void BeginPlay() override;

public:
    // Events the UI binds to

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnInventoryUpdated OnInventoryUpdated;       // Slots / selection / workbench changed.

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnCraftSuccess OnCraftSuccess;               // A recipe just matched.

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemPickedUp OnItemPickedUp;               // World pickup added to inventory.

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemCrafted OnItemCrafted;                 // Crafted item granted to the player.

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnPuzzleCraftRequested OnPuzzleCraftRequested; // Recipe requires solving a puzzle first.

    // State

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State")
    bool bHasEverPickedUpItem = false;            // Used to fire onetime tutorials on first pickup

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FInventorySlot> InventorySlots;        // Backing storage, fixed size after BeginPlay

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Layout")
    int32 SlotCount = 6;                          // Total slots created on BeginPlay

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Layout")
    int32 GridColumns = 2;                        // Columns used for grid based stick navigation

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|Selection")
    int32 SelectedSlotIndex = 0;                  // Slot currently highlighted in the UI

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
    TObjectPtr<UDataTable> RecipeDataTable;       // All recipes (rows of FCraftingRecipe)

    // Controller rumble feedback on failed craft
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Haptics")
    float CraftFailHapticIntensity = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Haptics")
    float CraftFailHapticDuration = 0.2f;

    //API

    UFUNCTION(BlueprintPure, Category = "Inventory|State")
    bool IsWorkbenchOpen() const { return bIsWorkbenchOpen; }

    UFUNCTION(BlueprintCallable, Category = "Inventory|State")
    void SetWorkbenchOpen(bool bOpen);

    UFUNCTION(BlueprintPure, Category = "Inventory|State")
    bool CanToggleInventoryUI() const { return !bIsWorkbenchOpen; }

    // Move the selection cursor horizontally (positive = right)
    UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
    void MoveSelection(int32 Direction);

    // Move the selection cursor in a 2D grid, skipping empty slots
    UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
    void MoveSelectionGrid(int32 DeltaX, int32 DeltaY);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
    void SetSelectedSlot(int32 NewIndex);

    // Snap the cursor to the first occupied slot (used after removing items)
    UFUNCTION(BlueprintCallable, Category = "Inventory|Selection")
    void SelectFirstAvailableSlot();

    // Toggle whether the currently selected slot is placed on the workbench
    UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
    void ToggleItemOnWorkbench();

    // Try to match the items on the workbench against any recipe. Fires events on success or failure
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void CraftItem();

    // Remove all items from the workbench (back into their slots)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearWorkbench();

    // Add a world pickup to the first empty slot. Returns false if the inventory is full
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItemToInventory(FName ItemToAdd, int32 Quantity);

    // Grant a crafted item directly (used by puzzle widgets after the puzzle is finished)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddCraftedItem(FName ItemID);

    // Remove every slot containing this item ID
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItemByID(FName ItemID);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsSlotOccupied(int32 SlotIndex) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsSlotSelected(int32 SlotIndex) const { return SlotIndex == SelectedSlotIndex; }

private:
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State",
              meta=(AllowPrivateAccess="true"))
    bool bIsWorkbenchOpen = false;
};
