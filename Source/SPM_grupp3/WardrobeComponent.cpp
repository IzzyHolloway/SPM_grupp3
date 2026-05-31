#include "WardrobeComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

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

    // Show a pickup notification (first-time vs subsequent) whenever a coat is unlocked.
    if (!OnCoatUnlocked.IsAlreadyBound(this, &UWardrobeComponent::HandleCoatUnlockedNotify))
    {
        OnCoatUnlocked.AddDynamic(this, &UWardrobeComponent::HandleCoatUnlockedNotify);
    }
}

void UWardrobeComponent::HandleCoatUnlockedNotify(FName CoatID, bool bFirstCoatEver)
{
    const TSubclassOf<UUserWidget> WidgetClass =
        bFirstCoatEver ? FirstCoatUnlockedWidgetClass : CoatUnlockedWidgetClass;
    if (!WidgetClass) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;

    // Replace any notification still on screen so they don't stack.
    RemoveNotifyWidget();

    ActiveNotifyWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (!ActiveNotifyWidget) return;

    ActiveNotifyWidget->AddToViewport();

    if (PickupNotifyDuration > 0.f)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                NotifyTimerHandle, this, &UWardrobeComponent::RemoveNotifyWidget, PickupNotifyDuration, false);
        }
    }
}

void UWardrobeComponent::RemoveNotifyWidget()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(NotifyTimerHandle);
    }
    if (ActiveNotifyWidget)
    {
        ActiveNotifyWidget->RemoveFromParent();
        ActiveNotifyWidget = nullptr;
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

// Navigates the cursor over the visible packed positions. The widget packs unlocked
// coats into the first N grid cells and leaves the rest empty, so navigation must
// operate on the packed position (0..Unlocked.Num()-1), not the raw slot index.
void UWardrobeComponent::MoveSelectionGrid(int32 DeltaX, int32 DeltaY)
{
    if (!bIsWardrobeOpen) return;

    TArray<int32> Unlocked;
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].bUnlocked) Unlocked.Add(i);
    }
    if (Unlocked.Num() == 0) return;

    int32 CurPos = Unlocked.IndexOfByKey(SelectedSlotIndex);
    if (CurPos == INDEX_NONE)
    {
        SelectedSlotIndex = Unlocked[0];
        OnWardrobeUpdated.Broadcast();
        return;
    }
    if (Unlocked.Num() == 1) return;

    const int32 Cols = FMath::Max(1, GridColumns);
    const int32 Rows = FMath::DivideAndRoundUp(Unlocked.Num(), Cols);
    const int32 CurRow = CurPos / Cols;
    const int32 CurCol = CurPos % Cols;

    int32 BestPos = CurPos;

    if (DeltaX != 0 && DeltaY == 0)
    {
        const int32 TargetCol = CurCol + DeltaX;
        if (TargetCol >= 0 && TargetCol < Cols)
        {
            const int32 Candidate = CurRow * Cols + TargetCol;
            if (Candidate < Unlocked.Num())
            {
                BestPos = Candidate;
            }
        }
    }
    else if (DeltaY != 0 && DeltaX == 0)
    {
        const int32 TargetRow = CurRow + DeltaY;
        if (TargetRow >= 0 && TargetRow < Rows)
        {
            int32 Candidate = TargetRow * Cols + CurCol;
            // Last row may be partially filled — snap to the last filled position.
            if (Candidate >= Unlocked.Num())
            {
                Candidate = Unlocked.Num() - 1;
            }
            BestPos = Candidate;
        }
    }

    if (BestPos != CurPos)
    {
        SelectedSlotIndex = Unlocked[BestPos];
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

void UWardrobeComponent::MarkGamepadNavInput()
{
    LastGamepadNavTime = FPlatformTime::Seconds();
}

bool UWardrobeComponent::CanMouseHoverChangeSelection() const
{
    return (FPlatformTime::Seconds() - LastGamepadNavTime) >= MouseHoverGamepadCooldown;
}
