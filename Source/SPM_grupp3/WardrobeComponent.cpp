#include "WardrobeComponent.h"

UWardrobeComponent::UWardrobeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SelectedSlotIndex = 0;
}

void UWardrobeComponent::BeginPlay()
{
    Super::BeginPlay();

    // Seed slots from the data table. Slot order follows row iteration order so the UI is
    // stable as long as rows are not reordered in the editor.
    Slots.Reset();
    if (CoatDataTable)
    {
        TArray<FName> RowNames = CoatDataTable->GetRowNames();
        Slots.Reserve(RowNames.Num());

        for (const FName& RowName : RowNames)
        {
            const FCoatDetail* Row = CoatDataTable->FindRow<FCoatDetail>(RowName, TEXT("Wardrobe Seed"));
            if (!Row) continue;

            FWardrobeSlot Slot;
            Slot.CoatID = RowName;
            Slot.bUnlocked = Row->bUnlockedByDefault;
            Slots.Add(Slot);

            // The first default-unlocked coat we see becomes Lumi's starting outfit.
            if (Row->bUnlockedByDefault && EquippedCoatID.IsNone())
            {
                EquippedCoatID = RowName;
            }
        }
    }

    SelectEquippedSlot();

    if (!EquippedCoatID.IsNone())
    {
        OnEquippedCoatChanged.Broadcast(EquippedCoatID);
    }
}

void UWardrobeComponent::SetWardrobeOpen(bool bOpen)
{
    if (bIsWardrobeOpen == bOpen) return;
    bIsWardrobeOpen = bOpen;
    OnWardrobeUpdated.Broadcast();
}

bool UWardrobeComponent::UnlockCoat(FName CoatID)
{
    if (CoatID.IsNone()) return false;

    const int32 Index = FindSlotIndexForCoat(CoatID);
    if (Index == INDEX_NONE) return false;
    if (Slots[Index].bUnlocked) return false;

    Slots[Index].bUnlocked = true;

    const bool bFirstCoatEver = !bHasEverUnlockedCoat;
    bHasEverUnlockedCoat = true;

    OnWardrobeUpdated.Broadcast();
    OnCoatUnlocked.Broadcast(CoatID, bFirstCoatEver);
    return true;
}

void UWardrobeComponent::EquipSelected()
{
    if (!Slots.IsValidIndex(SelectedSlotIndex)) return;

    const FWardrobeSlot& Slot = Slots[SelectedSlotIndex];
    if (!Slot.bUnlocked) return;

    EquipCoat(Slot.CoatID);
}

bool UWardrobeComponent::EquipCoat(FName CoatID)
{
    if (CoatID.IsNone()) return false;
    if (CoatID == EquippedCoatID) return false;

    const int32 Index = FindSlotIndexForCoat(CoatID);
    if (Index == INDEX_NONE) return false;
    if (!Slots[Index].bUnlocked) return false;

    EquippedCoatID = CoatID;
    OnEquippedCoatChanged.Broadcast(EquippedCoatID);
    OnWardrobeUpdated.Broadcast();
    return true;
}

// Moves the selection cursor across a 2D grid of slots, skipping locked ones.
// Same shape as UInventoryComponent::MoveSelectionGrid so the WBP can navigate the
// same way it does for the inventory.
void UWardrobeComponent::MoveSelectionGrid(int32 DeltaX, int32 DeltaY)
{
    if (!bIsWardrobeOpen) return;

    TArray<int32> Unlocked;
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].bUnlocked) Unlocked.Add(i);
    }
    if (Unlocked.Num() == 0) return;

    // If the cursor sits on a locked slot, jump to the first unlocked one.
    if (!Unlocked.Contains(SelectedSlotIndex))
    {
        SelectedSlotIndex = Unlocked[0];
        OnWardrobeUpdated.Broadcast();
        return;
    }
    if (Unlocked.Num() == 1) return;

    const int32 Cols = FMath::Max(1, GridColumns);
    const int32 Rows = FMath::DivideAndRoundUp(Slots.Num(), Cols);
    const int32 CurRow = SelectedSlotIndex / Cols;
    const int32 CurCol = SelectedSlotIndex % Cols;

    int32 Best = SelectedSlotIndex;

    if (DeltaX != 0 && DeltaY == 0)
    {
        for (int32 Step = 1; Step < Cols; ++Step)
        {
            const int32 TargetCol = CurCol + DeltaX * Step;
            if (TargetCol < 0 || TargetCol >= Cols) break;

            const int32 Candidate = CurRow * Cols + TargetCol;
            if (Candidate >= Slots.Num()) break;
            if (Slots[Candidate].bUnlocked)
            {
                Best = Candidate; break;
            }
        }
    }
    else if (DeltaY != 0 && DeltaX == 0)
    {
        for (int32 Step = 1; Step < Rows; ++Step)
        {
            const int32 TargetRow = CurRow + DeltaY * Step;
            if (TargetRow < 0 || TargetRow >= Rows) break;

            const int32 SameCol = TargetRow * Cols + CurCol;
            if (SameCol < Slots.Num() && Slots[SameCol].bUnlocked)
            {
                Best = SameCol; break;
            }

            bool bFound = false;
            for (int32 Col = 0; Col < Cols; ++Col)
            {
                const int32 Candidate = TargetRow * Cols + Col;
                if (Candidate < Slots.Num()
                    && Slots[Candidate].bUnlocked
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
        OnWardrobeUpdated.Broadcast();
    }
}

void UWardrobeComponent::MoveSelection(int32 Direction)
{
    MoveSelectionGrid(Direction, 0);
}

void UWardrobeComponent::SetSelectedSlot(int32 NewIndex)
{
    if (!bIsWardrobeOpen) return;
    if (!IsSlotUnlocked(NewIndex)) return;
    if (NewIndex == SelectedSlotIndex) return;
    SelectedSlotIndex = NewIndex;
    OnWardrobeUpdated.Broadcast();
}

void UWardrobeComponent::SelectEquippedSlot()
{
    if (!EquippedCoatID.IsNone())
    {
        const int32 Index = FindSlotIndexForCoat(EquippedCoatID);
        if (Index != INDEX_NONE)
        {
            SelectedSlotIndex = Index;
            OnWardrobeUpdated.Broadcast();
            return;
        }
    }

    // Fallback: first unlocked slot.
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].bUnlocked)
        {
            SelectedSlotIndex = i;
            OnWardrobeUpdated.Broadcast();
            return;
        }
    }

    SelectedSlotIndex = 0;
    OnWardrobeUpdated.Broadcast();
}

bool UWardrobeComponent::IsSlotUnlocked(int32 SlotIndex) const
{
    return Slots.IsValidIndex(SlotIndex) && Slots[SlotIndex].bUnlocked;
}

bool UWardrobeComponent::IsSlotEquipped(int32 SlotIndex) const
{
    return Slots.IsValidIndex(SlotIndex) && Slots[SlotIndex].CoatID == EquippedCoatID;
}

bool UWardrobeComponent::IsCoatUnlocked(FName CoatID) const
{
    const int32 Index = FindSlotIndexForCoat(CoatID);
    return Index != INDEX_NONE && Slots[Index].bUnlocked;
}

int32 UWardrobeComponent::FindSlotIndexForCoat(FName CoatID) const
{
    if (CoatID.IsNone()) return INDEX_NONE;
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].CoatID == CoatID) return i;
    }
    return INDEX_NONE;
}
