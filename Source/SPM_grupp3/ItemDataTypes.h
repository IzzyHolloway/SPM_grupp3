// Shared data types for the inventory and crafting system.

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDataTypes.generated.h"

class UUserWidget;


USTRUCT(BlueprintType)
struct FItemDetail: public FTableRowBase
{
	GENERATED_BODY()

	public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FText ItemName;          // Display name shown in UI.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	UTexture2D* ItemIcon;    // Icon shown in inventory slots.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 MaxStackSize;      // Max items per slot (1 = unstackable).

};

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName ItemID;            // NAME_None means the slot is empty.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 ItemQuantity;      // How many of this item the slot holds.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	bool bIsOnWorkbench;     // True if the player placed this slot on the workbench.


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 WorkbenchOrder = -1; // Placement order on the workbench (-1 = not placed).

	FInventorySlot()
	{
		ItemID = NAME_None;
		ItemQuantity = 0;
		bIsOnWorkbench = false;
		WorkbenchOrder = -1;
	}
};

USTRUCT(BlueprintType)
struct FCraftingRecipe : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FName> RequiredIngredients;   // Item IDs that must be on the workbench.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FName ResultItemID;                  // Item produced when the recipe matches.


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Progression")
	FName ProgressionFlagToAdd = NAME_None;        // Optional story flag set on success.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Puzzle")
	TSubclassOf<UUserWidget> PuzzleWidgetClass;    // If set, opens this puzzle instead of granting the item directly.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	bool bOrderMatters = false;          // If true, ingredients must be placed in the exact order listed.
};
