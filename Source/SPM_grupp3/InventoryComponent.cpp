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
    InventorySlots.Init(FInventorySlot(), 6);
}

bool UInventoryComponent::IsSlotOccupied(int32 SlotIndex) const
{
    return InventorySlots.IsValidIndex(SlotIndex)
        && InventorySlots[SlotIndex].ItemID != NAME_None;
}

void UInventoryComponent::MoveSelection(int32 Direction)
{
    MoveSelectionGrid(Direction, 0);
}

void UInventoryComponent::MoveSelectionGrid(int32 DeltaX, int32 DeltaY)
{
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
        const int32 N = Occupied.Num();
        const int32 Idx = Occupied.IndexOfByKey(SelectedSlotIndex);
        Best = Occupied[(Idx + DeltaX + N) % N];
    }
    else if (DeltaY != 0 && DeltaX == 0)
    {
        for (int32 Step = 1; Step <= Rows; ++Step)
        {
            const int32 TargetRow = (CurRow + DeltaY * Step + Rows) % Rows;

            const int32 SameCol = TargetRow * Cols + CurCol;
            if (SameCol < InventorySlots.Num() && InventorySlots[SameCol].ItemID != NAME_None)
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
        if (Slot.bIsOnWorkbench) { Slot.bIsOnWorkbench = false; bAny = true; }
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
            ItemsOnBench.Add(Slot.ItemID);
    }
    if (ItemsOnBench.Num() == 0) return;

    ItemsOnBench.Sort([](const FName& A, const FName& B){ return A.ToString() < B.ToString(); });

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

        RecipeIngredients.Sort([](const FName& A, const FName& B){ return A.ToString() < B.ToString(); });

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

    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.bIsOnWorkbench)
        {
            Slot.ItemID = NAME_None;
            Slot.ItemQuantity = 0;
            Slot.bIsOnWorkbench = false;
        }
    }

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

    SelectFirstAvailableSlot();
    OnInventoryUpdated.Broadcast();

    if (!ProgressionFlagToAdd.IsNone())
    {
        if (AProgressionManager* PM = Cast<AProgressionManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())))
        {
            PM->AddFlag(ProgressionFlagToAdd);
            UE_LOG(LogTemp, Warning, TEXT("Crafting added progression flag: %s"), *ProgressionFlagToAdd.ToString());
        }
    }
}

bool UInventoryComponent::AddItemToInventory(FName ItemToAdd, int32 Quantity)
{
    for (int32 i = 0; i < 6; ++i)
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