#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"
#include "InventoryComponent.h"
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

    ItemsOnBench.Sort([](const FName& A, const FName& B) {
        return A.ToString() < B.ToString();
    });

    TArray<FCraftingRecipe*> Recipes;
    RecipeDataTable->GetAllRows<FCraftingRecipe>(TEXT("Crafting Context"), Recipes);

    bool bCraftingSuccess = false;
    FName ResultingItem = NAME_None;
   // ProgressionFlag adding
   FName ProgressionFlagToAdd = NAME_None;

    for (FCraftingRecipe* Recipe : Recipes)
    {
       if (!Recipe) continue;

       TArray<FName> RecipeIngredients = Recipe->RequiredIngredients;

       if (RecipeIngredients.Num() != ItemsOnBench.Num()) continue;

       RecipeIngredients.Sort([](const FName& A, const FName& B) {
           return A.ToString() < B.ToString();
       });

       bool bMatch = true;
       for (int32 i = 0; i < ItemsOnBench.Num(); ++i)
       {
          if (ItemsOnBench[i] != RecipeIngredients[i])
          {
             bMatch = false; 
             break;
          }
       }
       
       if (bMatch)
       {
          bCraftingSuccess = true;
          ResultingItem = Recipe->ResultItemID;
          // ProgressionFlag
          ProgressionFlagToAdd = Recipe->ProgressionFlagToAdd;
          break; 
       }
    }

    if (bCraftingSuccess)
    {
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
             break; 
          }
       }
       
       if (!ProgressionFlagToAdd.IsNone())
       {
          AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
             UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
          );

          if (ProgressionManager)
          {
             ProgressionManager->AddFlag(ProgressionFlagToAdd);

             UE_LOG(LogTemp, Warning, TEXT("Crafting added progression flag: %s"), *ProgressionFlagToAdd.ToString());
          }
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
         return true;
      }
   }
   return false;
}