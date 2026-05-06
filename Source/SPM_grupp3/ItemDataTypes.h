#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDataTypes.generated.h"

USTRUCT(BlueprintType)
struct FItemDetail: public FTableRowBase
{
	GENERATED_BODY()
	
	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	UTexture2D* ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 MaxStackSize;
	
};

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName ItemID; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 ItemQuantity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	bool bIsOnWorkbench;

	// se till att en tom slot är nollställd
	FInventorySlot()
	{
		ItemID = NAME_None;
		ItemQuantity = 0;
		bIsOnWorkbench = false; // Som standard ligger inget på workbench
	}
};
