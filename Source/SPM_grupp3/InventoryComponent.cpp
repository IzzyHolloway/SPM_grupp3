#include "InventoryComponent.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"
#include "ItemDataTypes.h"

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

    InventorySlots[SelectedSlotIndex].bIsOnWorkbench =
        !InventorySlots[SelectedSlotIndex].bIsOnWorkbench;
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
            bAny = true;
        }
    }
    if (bAny) OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::CraftItem()
{
    if (!RecipeDataTable) return;

    TArray<FName> ItemsOnBench;
    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.bIsOnWorkbench && Slot.ItemID != NAME_None)
        {
            ItemsOnBench.Add(Slot.ItemID);
        }
    }
    if (ItemsOnBench.Num() == 0) return;

    ItemsOnBench.Sort([](const FName& A, const FName& B)
    {
        return A.ToString() < B.ToString();
    });

    TArray<FCraftingRecipe*> Recipes;
    RecipeDataTable->GetAllRows<FCraftingRecipe>(TEXT("Crafting Context"), Recipes);

    bool bSuccess = false;
    FName ResultingItem = NAME_None;
    FName ProgressionFlagToAdd = NAME_None;

    for (FCraftingRecipe* Recipe : Recipes)
    {
        if (!Recipe) continue;

        TArray<FName> RecipeIngredients = Recipe->RequiredIngredients;
        if (RecipeIngredients.Num() != ItemsOnBench.Num()) continue;

        RecipeIngredients.Sort([](const FName& A, const FName& B)
        {
            return A.ToString() < B.ToString();
        });

        bool bMatch = true;
        for (int32 i = 0; i < ItemsOnBench.Num(); ++i)
        {
            if (ItemsOnBench[i] != RecipeIngredients[i]) { bMatch = false; break; }
        }

        if (bMatch)
        {
            bSuccess = true;
            ResultingItem = Recipe->ResultItemID;
            ProgressionFlagToAdd = Recipe->ProgressionFlagToAdd;
            break;
        }
    }

    if (!bSuccess) return;

    // Consume ingredients.
    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.bIsOnWorkbench)
        {
            Slot.ItemID = NAME_None;
            Slot.ItemQuantity = 0;
            Slot.bIsOnWorkbench = false;
        }
    }

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
            OnInventoryUpdated.Broadcast();
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
