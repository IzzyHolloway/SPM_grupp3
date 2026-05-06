#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemDataTypes.h" 
#include "InventoryComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FInventorySlot> InventorySlots;
	
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Selection")
	int32 SelectedSlotIndex;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
	void MoveSelection(int32 Direction);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Input")
	void ToggleItemOnWorkbench();
};