#include "InventoryComponent.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"
#include "ItemDataTypes.h"
#include "Blueprint/UserWidget.h" //Izzy lagt till för ritpussel (TSubclassOf<UUserWidget>)

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SelectedSlotIndex = 0;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    InventorySlots.Init(FInventorySlot(), FMath::Max(1, SlotCount));
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

void UInventoryComponent::MoveSelectionGrid(int32 DeltaX, int32 DeltaY)
{
    if (!bIsWorkbenchOpen) return;

    TArray<int32> Occupied;
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (InventorySlots[i].ItemID != NAME_None) Occupied.Add(i);
    }
    if (Occupied.Num() == 0) return;

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
        // Steg vänster/höger i samma rad, stoppa vid kant.
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
        // Steg upp/ner i samma kolumn, stoppa vid kant.
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

void UInventoryComponent::ToggleItemOnWorkbench()
{
    if (!bIsWorkbenchOpen) return;
    if (!InventorySlots.IsValidIndex(SelectedSlotIndex)) return;
    if (InventorySlots[SelectedSlotIndex].ItemID == NAME_None) return;

    FInventorySlot& Slot = InventorySlots[SelectedSlotIndex];
    Slot.bIsOnWorkbench = !Slot.bIsOnWorkbench;

    if (Slot.bIsOnWorkbench)
    {
        // Assign the next available WorkbenchOrder so we know where in the sequence
        // this item was placed. Used by order-sensitive recipes.
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
        // Removed from bench — clear order.
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
            Slot.WorkbenchOrder = -1; //Izzy lagt till för ordnings-recept
            bAny = true;
        }
    }
    if (bAny) OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::CraftItem()
{
    if (!RecipeDataTable) return;

    // //Izzy lagt till för ordnings-recept
    // Collect items WITH their placement order so we can support order-sensitive recipes.
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

    // Sort by placement order — this is the "as-placed" view used for ordered recipes.
    BenchEntries.Sort([](const FBenchEntry& A, const FBenchEntry& B)
    {
        return A.Order < B.Order;
    });

    TArray<FName> ItemsInOrder;
    ItemsInOrder.Reserve(BenchEntries.Num());
    for (const FBenchEntry& BE : BenchEntries) ItemsInOrder.Add(BE.ItemID);

    // Alphabetical view used for unordered recipes (existing behavior).
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
    TSubclassOf<UUserWidget> PuzzleWidgetClass = nullptr; //Izzy lagt till för ritpussel

    for (FCraftingRecipe* Recipe : Recipes)
    {
        if (!Recipe) continue;
        if (Recipe->RequiredIngredients.Num() != ItemsInOrder.Num()) continue;

        bool bMatch = true;

        //Izzy lagt till för ordnings-recept
        if (Recipe->bOrderMatters)
        {
            // Strict order: compare item-by-item in placement order.
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
            // Unordered: sort both and compare (preserves existing behavior).
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
            PuzzleWidgetClass = Recipe->PuzzleWidgetClass; //Izzy lagt till för ritpussel
            break;
        }
    }

    if (!bSuccess)
    {
        //Izzy lagt till för craft-fail-haptik
        // Light controller buzz to signal "that didn't work" — no recipe matched.
        if (CraftFailHapticIntensity > 0.f)
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
            {
                FForceFeedbackParameters Params;
                Params.bLooping = false;
                Params.Tag = TEXT("CraftFail");
                // PlayDynamicForceFeedback fires all four motors lightly for Duration seconds.
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

        //Izzy lagt till — vid misslyckad craft, släng tillbaka items från workbenchen
        // till inventoryt så spelaren måste placera dem igen. Bra för pussel-recept där
        // ordningen var fel — då tvingas spelaren tänka om från början.
        ClearWorkbench();
        SelectFirstAvailableSlot(); // välj första item så cursor inte hänger på tom slot

        return;
    }

    // Consume ingredients.
    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.bIsOnWorkbench)
        {
            Slot.ItemID = NAME_None;
            Slot.ItemQuantity = 0;
            Slot.bIsOnWorkbench = false;
            Slot.WorkbenchOrder = -1; //Izzy lagt till för ordnings-recept
        }
    }

    // Puzzle crafts skip placing the result in the inventory — the puzzle widget
    // grants the item itself when the player finishes the puzzle.
    const bool bIsPuzzleCraft = (PuzzleWidgetClass != nullptr);

    if (!bIsPuzzleCraft)
    {
        // Place result in first empty slot.
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
        // Don't broadcast OnItemCrafted yet — the puzzle widget does that via AddCraftedItem.
        OnPuzzleCraftRequested.Broadcast(ResultingItem, PuzzleWidgetClass);
    }
    else
    {
        OnItemCrafted.Broadcast(ResultingItem);
    }
}

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

            OnInventoryUpdated.Broadcast();
            OnItemCrafted.Broadcast(ItemID);
            return true;
        }
    }
    return false;
}

bool UInventoryComponent::AddItemToInventory(FName ItemToAdd, int32 Quantity)
{
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (InventorySlots[i].ItemID == NAME_None)
        {
            InventorySlots[i].ItemID = ItemToAdd;
            InventorySlots[i].ItemQuantity = Quantity;
            InventorySlots[i].bIsOnWorkbench = false;

            const bool bFirstPickupEver = !bHasEverPickedUpItem;
            bHasEverPickedUpItem = true;

            OnInventoryUpdated.Broadcast();
            OnItemPickedUp.Broadcast(ItemToAdd, bFirstPickupEver);
            return true;
        }
    }
    return false;
}

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
            bRemoved = true;
        }
    }

    if (bRemoved)
    {
        // Selected slot may now be empty; reselect first occupied (or 0).
        SelectFirstAvailableSlot(); // broadcasts OnInventoryUpdated
    }

    return bRemoved;
}
