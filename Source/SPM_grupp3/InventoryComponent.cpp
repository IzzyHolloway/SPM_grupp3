
#include "InventoryComponent.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"
#include "ItemDataTypes.h"
#include "Blueprint/UserWidget.h" // Needed for TSubclassOf<UUserWidget> in the puzzle-craft path

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SelectedSlotIndex = 0;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // Allocate the fixed number of slots so the array index can be used as the UI slot index.
    //
    // IMPORTANT: only initialise when the array isn't already the expected size. On a level
    // transition the GameInstance (ApplyToWorld) restores the saved InventorySlots, and that
    // restore can run BEFORE this BeginPlay. A saved/restored inventory always has exactly
    // SlotCount entries, so if we see the right size we leave it alone -- re-initialising here
    // would wipe the carried-over items and we'd lose the inventory between levels.
    const int32 DesiredSize = FMath::Max(1, SlotCount);
    if (InventorySlots.Num() != DesiredSize)
    {
        InventorySlots.Init(FInventorySlot(), DesiredSize);
    }
}

bool UInventoryComponent::IsSlotOccupied(int32 SlotIndex) const
{
    return InventorySlots.IsValidIndex(SlotIndex)
        && InventorySlots[SlotIndex].ItemID != NAME_None;
}

void UInventoryComponent::SetWorkbenchOpen(bool bOpen)
{
    if (bIsWorkbenchOpen == bOpen) return;
    bIsWorkbenchOpen = bOpen;
    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::MoveSelection(int32 Direction)
{
    MoveSelectionGrid(Direction, 0);
}

// Moves the selection cursor across a 2D grid of slots, skipping empty ones
void UInventoryComponent::MoveSelectionGrid(int32 DeltaX, int32 DeltaY)
{
    if (!bIsWorkbenchOpen) return;

    // Collects occupied slot indices once so we don't recompute them in the loops below
    TArray<int32> Occupied;
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (InventorySlots[i].ItemID != NAME_None) Occupied.Add(i);
    }
    if (Occupied.Num() == 0) return;

    // If the current selection sits on an empty slot, jump to the first occupied one
    if (!Occupied.Contains(SelectedSlotIndex))
    {
        SelectedSlotIndex = Occupied[0];
        OnInventoryUpdated.Broadcast();
        return;
    }
    if (Occupied.Num() == 1) return;

    const int32 Cols = FMath::Max(1, GridColumns);
    const int32 Rows = FMath::DivideAndRoundUp(InventorySlots.Num(), Cols);
    const int32 CurRow = SelectedSlotIndex / Cols;
    const int32 CurCol = SelectedSlotIndex % Cols;

    int32 Best = SelectedSlotIndex;

    if (DeltaX != 0 && DeltaY == 0)
    {
        // Step left/right within the same row, stop at the edge
        for (int32 Step = 1; Step < Cols; ++Step)
        {
            const int32 TargetCol = CurCol + DeltaX * Step;
            if (TargetCol < 0 || TargetCol >= Cols) break;

            const int32 Candidate = CurRow * Cols + TargetCol;
            if (Candidate >= InventorySlots.Num()) break;
            if (InventorySlots[Candidate].ItemID != NAME_None)
            {
                Best = Candidate; break;
            }
        }
    }
    else if (DeltaY != 0 && DeltaX == 0)
    {
        // Step up/down. Prefer the same column; if empty, scan the row for any occupied slot
        for (int32 Step = 1; Step < Rows; ++Step)
        {
            const int32 TargetRow = CurRow + DeltaY * Step;
            if (TargetRow < 0 || TargetRow >= Rows) break;

            const int32 SameCol = TargetRow * Cols + CurCol;
            if (SameCol < InventorySlots.Num() &&
                InventorySlots[SameCol].ItemID != NAME_None)
            {
                Best = SameCol; break;
            }

            bool bFound = false;
            for (int32 Col = 0; Col < Cols; ++Col)
            {
                const int32 Candidate = TargetRow * Cols + Col;
                if (Candidate < InventorySlots.Num()
                    && InventorySlots[Candidate].ItemID != NAME_None
                    && Candidate != SelectedSlotIndex)
                {
                    Best = Candidate; bFound = true; break;
                }
            }
            if (bFound) break;
        }
    }

    if (Best != SelectedSlotIndex)
    {
        SelectedSlotIndex = Best;
        OnInventoryUpdated.Broadcast();
    }
}

void UInventoryComponent::SetSelectedSlot(int32 NewIndex)
{
    if (!bIsWorkbenchOpen) return;
    if (!IsSlotOccupied(NewIndex)) return;
    SelectedSlotIndex = NewIndex;
    OnInventoryUpdated.Broadcast();
}

// Snaps the cursor to the first occupied slot, used after removing items so the cursor
// doesn't get stuck on an empty slot.
void UInventoryComponent::SelectFirstAvailableSlot()
{
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (InventorySlots[i].ItemID != NAME_None)
        {
            SelectedSlotIndex = i;
            OnInventoryUpdated.Broadcast();
            return;
        }
    }
    SelectedSlotIndex = 0;
    OnInventoryUpdated.Broadcast();
}

// Places or removes the selected item on the workbench. Assigns a placement order
// so order sensitive recipes can match against the sequence the player placed items in.
void UInventoryComponent::ToggleItemOnWorkbench()
{
    if (!bIsWorkbenchOpen) return;
    if (!InventorySlots.IsValidIndex(SelectedSlotIndex)) return;
    if (InventorySlots[SelectedSlotIndex].ItemID == NAME_None) return;

    FInventorySlot& Slot = InventorySlots[SelectedSlotIndex];
    Slot.bIsOnWorkbench = !Slot.bIsOnWorkbench;

    if (Slot.bIsOnWorkbench)
    {
        // Assign the next placement order (existing max + 1)
        int32 MaxOrder = -1;
        for (const FInventorySlot& Other : InventorySlots)
        {
            if (Other.bIsOnWorkbench && Other.WorkbenchOrder > MaxOrder)
            {
                MaxOrder = Other.WorkbenchOrder;
            }
        }
        Slot.WorkbenchOrder = MaxOrder + 1;
    }
    else
    {
        // Removed from bench — clear order
        Slot.WorkbenchOrder = -1;
    }

    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::ClearWorkbench()
{
    bool bAny = false;
    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.bIsOnWorkbench)
        {
            Slot.bIsOnWorkbench = false;
            Slot.WorkbenchOrder = -1;
            bAny = true;
        }
    }
    if (bAny) OnInventoryUpdated.Broadcast();
}

// Tries to match the items on the workbench against any recipe in the data table.
// On success, consumes the ingredients and either places the result in inventory
// or opens a puzzle widget (for puzzle-style crafts)
void UInventoryComponent::CraftItem()
{
    if (!RecipeDataTable) return;

    // Collect items WITH their placement order so we can support order-sensitive recipes
    struct FBenchEntry
    {
        FName ItemID;
        int32 Order;
    };
    TArray<FBenchEntry> BenchEntries;
    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.bIsOnWorkbench && Slot.ItemID != NAME_None)
        {
            BenchEntries.Add({Slot.ItemID, Slot.WorkbenchOrder});
        }
    }
    if (BenchEntries.Num() == 0) return;

    // Sort by placement order, this is the "as-placed" view used for ordered recipes
    BenchEntries.Sort([](const FBenchEntry& A, const FBenchEntry& B)
    {
        return A.Order < B.Order;
    });

    TArray<FName> ItemsInOrder;
    ItemsInOrder.Reserve(BenchEntries.Num());
    for (const FBenchEntry& BE : BenchEntries) ItemsInOrder.Add(BE.ItemID);

    // Alphabetical view used for unordered recipes
    TArray<FName> ItemsAlpha = ItemsInOrder;
    ItemsAlpha.Sort([](const FName& A, const FName& B)
    {
        return A.ToString() < B.ToString();
    });

    TArray<FCraftingRecipe*> Recipes;
    RecipeDataTable->GetAllRows<FCraftingRecipe>(TEXT("Crafting Context"), Recipes);

    bool bSuccess = false;
    FName ResultingItem = NAME_None;
    FName ProgressionFlagToAdd = NAME_None;
    TSubclassOf<UUserWidget> PuzzleWidgetClass = nullptr;

    // Find the first recipe whose ingredients match the workbench
    for (FCraftingRecipe* Recipe : Recipes)
    {
        if (!Recipe) continue;
        if (Recipe->RequiredIngredients.Num() != ItemsInOrder.Num()) continue;

        bool bMatch = true;

        if (Recipe->bOrderMatters)
        {
            // Strict order: compare item-by-item in placement order
            for (int32 i = 0; i < ItemsInOrder.Num(); ++i)
            {
                if (ItemsInOrder[i] != Recipe->RequiredIngredients[i])
                {
                    bMatch = false;
                    break;
                }
            }
        }
        else
        {
            // Unordered: sort both lists and compare
            TArray<FName> RecipeSorted = Recipe->RequiredIngredients;
            RecipeSorted.Sort([](const FName& A, const FName& B)
            {
                return A.ToString() < B.ToString();
            });

            for (int32 i = 0; i < ItemsAlpha.Num(); ++i)
            {
                if (ItemsAlpha[i] != RecipeSorted[i]) { bMatch = false; break; }
            }
        }

        if (bMatch)
        {
            bSuccess = true;
            ResultingItem = Recipe->ResultItemID;
            ProgressionFlagToAdd = Recipe->ProgressionFlagToAdd;
            PuzzleWidgetClass = Recipe->PuzzleWidgetClass;
            break;
        }
    }

    if (!bSuccess)
    {
        // Light controller buzz so the player feels that the craft failed
        if (CraftFailHapticIntensity > 0.f)
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
            {
                FForceFeedbackParameters Params;
                Params.bLooping = false;
                Params.Tag = TEXT("CraftFail");

                PC->PlayDynamicForceFeedback(
                    CraftFailHapticIntensity,
                    CraftFailHapticDuration,
                    /*LeftLarge=*/  true,
                    /*LeftSmall=*/  true,
                    /*RightLarge=*/ true,
                    /*RightSmall=*/ true,
                    EDynamicForceFeedbackAction::Start);
            }
        }

        // Clear the workbench on failure so the player has to rethink the placement
        // important for order-sensitive recipes.
        ClearWorkbench();
        SelectFirstAvailableSlot();

        return;
    }

    // Consume ingredients
    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.bIsOnWorkbench)
        {
            Slot.ItemID = NAME_None;
            Slot.ItemQuantity = 0;
            Slot.bIsOnWorkbench = false;
            Slot.WorkbenchOrder = -1;
        }
    }

    // Puzzle crafts skip placing the result in the inventory, the puzzle widget
    // grants the item itself once the player finishes the puzzle
    const bool bIsPuzzleCraft = (PuzzleWidgetClass != nullptr);

    if (!bIsPuzzleCraft)
    {
        // Place the result in the first empty slot
        for (FInventorySlot& Slot : InventorySlots)
        {
            if (Slot.ItemID == NAME_None)
            {
                Slot.ItemID = ResultingItem;
                Slot.ItemQuantity = 1;
                Slot.bIsOnWorkbench = false;
                break;
            }
        }
    }

    SelectFirstAvailableSlot(); // broadcasts OnInventoryUpdated

    // Optional story flag, drives the progression system
    if (!ProgressionFlagToAdd.IsNone())
    {
        if (AProgressionManager* PM = Cast<AProgressionManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())))
        {
            PM->AddFlag(ProgressionFlagToAdd);
            UE_LOG(LogTemp, Warning, TEXT("Crafting added progression flag: %s"),
                   *ProgressionFlagToAdd.ToString());
        }
    }

    OnCraftSuccess.Broadcast();

    if (bIsPuzzleCraft)
    {
        // Don't broadcast OnItemCrafted yet, the puzzle widget will do that via AddCraftedItem
        // once the player finishes the puzzle
        OnPuzzleCraftRequested.Broadcast(ResultingItem, PuzzleWidgetClass);
    }
    else
    {
        OnItemCrafted.Broadcast(ResultingItem);
    }
}

// Grants a crafted item directly, Called by puzzle widgets after the puzzle is finished
bool UInventoryComponent::AddCraftedItem(FName ItemID)
{
    if (ItemID.IsNone()) return false;

    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.ItemID == NAME_None)
        {
            Slot.ItemID = ItemID;
            Slot.ItemQuantity = 1;
            Slot.bIsOnWorkbench = false;
            Slot.WorkbenchOrder = -1; // Aimi la till

            OnInventoryUpdated.Broadcast();
            OnItemCrafted.Broadcast(ItemID);
            return true;
        }
    }
    return false;
}

// Adds a world pickup to the first empty slot, Returns false if the inventory is full
bool UInventoryComponent::AddItemToInventory(FName ItemToAdd, int32 Quantity)
{
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (InventorySlots[i].ItemID == NAME_None)
        {
            InventorySlots[i].ItemID = ItemToAdd;
            InventorySlots[i].ItemQuantity = Quantity;
            InventorySlots[i].bIsOnWorkbench = false;
            InventorySlots[i].WorkbenchOrder = -1; // Aimi la till

            // Track first-ever pickup so we can fire one-time tutorials / hints
            const bool bFirstPickupEver = !bHasEverPickedUpItem;
            bHasEverPickedUpItem = true;

            OnInventoryUpdated.Broadcast();
            OnItemPickedUp.Broadcast(ItemToAdd, bFirstPickupEver);
            return true;
        }
    }
    return false;
}

// Removes every slot containing this item ID
bool UInventoryComponent::RemoveItemByID(FName ItemID)
{
    if (ItemID.IsNone()) return false;

    bool bRemoved = false;
    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.ItemID == ItemID)
        {
            Slot.ItemID = NAME_None;
            Slot.ItemQuantity = 0;
            Slot.bIsOnWorkbench = false;
            Slot.WorkbenchOrder = -1; // Aimi la till
            bRemoved = true;
        }
    }

    if (bRemoved)
    {
        // Selected slot may now be empty; reselect the first occupied (or 0)
        SelectFirstAvailableSlot();
    }

    return bRemoved;
}
