#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FName UInventoryComponent::CombineItems(FName InputA, FName InputB)
{
	for (const FCombineRecipe& Recipe : AllRecipes)
	{
		bool bMatch1 = (Recipe.ItemA == InputA && Recipe.ItemB == InputB);
		bool bMatch2 = (Recipe.ItemA == InputB && Recipe.ItemB == InputA);

		if (bMatch1 || bMatch2)
		{
			return Recipe.ResultItem; 
		}
	}

	return NAME_None; 
}