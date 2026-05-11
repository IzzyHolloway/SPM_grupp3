#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemDataTypes.h" 
#include "InventoryComponent.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:	
public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryUpdated OnInventoryUpdated;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FInventorySlot> InventorySlots;
	
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Selection")
	int32 SelectedSlotIndex;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
	void MoveSelection(int32 Direction);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
	void ToggleItemOnWorkbench();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
	class UDataTable* RecipeDataTable;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CraftItem();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItemToInventory(FName ItemToAdd, int32 Quantity);
};