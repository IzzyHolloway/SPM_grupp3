#include "InventoryComponent.h"

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


void UInventoryComponent::MoveSelection(int32 Direction)
{
	SelectedSlotIndex += Direction;
	
	if (SelectedSlotIndex > 5)
	{
		SelectedSlotIndex = 0;
	}
	else if (SelectedSlotIndex < 0)
	{
		SelectedSlotIndex = 5;
	}
}

void UInventoryComponent::ToggleItemOnWorkbench()
{
	if (InventorySlots.IsValidIndex(SelectedSlotIndex))
	{
		if (InventorySlots[SelectedSlotIndex].ItemID != NAME_None)
		{
			InventorySlots[SelectedSlotIndex].bIsOnWorkbench = !InventorySlots[SelectedSlotIndex].bIsOnWorkbench;
		}
	}
}