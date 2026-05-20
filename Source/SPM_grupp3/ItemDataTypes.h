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

	/**
	 * Order in which the item was placed on the workbench (0 = first, 1 = second, ...).
	 * -1 means "not on the workbench". Used for order-sensitive recipes
	 * (e.g. the colored notes puzzle on Island 1 where order matters).
	 *
	 * //Izzy lagt till för ordnings-recept
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 WorkbenchOrder = -1;

	// se till att en tom slot är nollställd
	FInventorySlot()
	{
		ItemID = NAME_None;
		ItemQuantity = 0;
		bIsOnWorkbench = false; // Som standard ligger inget på workbench
		WorkbenchOrder = -1;    // Inte placerad
	}
};

USTRUCT(BlueprintType)
struct FCraftingRecipe : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FName> RequiredIngredients;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FName ResultItemID;
	
	// Optional progression flag added when this recipe is crafted successfully.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Progression")
	FName ProgressionFlagToAdd = NAME_None;

	/**
	 * If set, the craft opens this widget instead of placing the result directly in the inventory.
	 * The widget is responsible for calling InventoryComponent::AddCraftedItem(ResultItemID)
	 * when the puzzle is completed. Used for "puzzle crafts" such as the Drawing puzzle.
	 *
	 * //Izzy lagt till för ritpussel
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Puzzle")
	TSubclassOf<UUserWidget> PuzzleWidgetClass;

	/**
	 * If true, RequiredIngredients must appear in EXACTLY this order on the workbench
	 * (according to the order the player placed them). Used for puzzles like the colored
	 * notes on Island 1 (Blue → Yellow → Green → Red). Default false = order doesn't matter.
	 *
	 * //Izzy lagt till för ordnings-recept
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	bool bOrderMatters = false;
};
