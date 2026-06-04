// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveManager.h"

#include "CharacterAimi.h"
#include "InventoryComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ObjectiveWidget.h"

// Sets default values
AObjectiveManager::AObjectiveManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AObjectiveManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Save pointer to inventory component corresponding to the possessed character
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController is null. Unable to access inventory component."));
		return;
	}
	ACharacterAimi* Character = Cast<ACharacterAimi>(PlayerController->GetCharacter());
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("Character is null. Unable to access inventory component."));
		return;
	}
	Inventory = Cast<UInventoryComponent>(Character->GetComponentByClass(UInventoryComponent::StaticClass()));
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to access inventory component."));
		return;
	}
	
	// Subscribe to needed events of inventory
	Inventory->OnItemPickedUp.AddDynamic(this, &AObjectiveManager::OnItemPickedUp);
	Inventory->OnItemCrafted.AddDynamic(this, &AObjectiveManager::OnItemCrafted);
}

// Called every frame
void AObjectiveManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AObjectiveManager::ShowObjectiveProgress(int32 CurrentProgress, int32 TotalProgress)
{
	if (TotalProgress <= 0)
	{
		HideObjective();
		return;
	}

	CurrentObjectiveNumber = FMath::Clamp(CurrentProgress, 0, TotalProgress);
	TotalObjectiveNumber = TotalProgress;

	// If the Objective Widget already exists, update it
	if (ObjectiveWidgetInstance)
	{
		ObjectiveWidgetInstance->UpdateProgressText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentObjectiveNumber, TotalObjectiveNumber)));
		return;
	}

	// Create the widget
	if (!ObjectiveWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Objective Widget Class is null. Unable to create the widget."));
		return;
	}
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
	{
		return;
	}
	ObjectiveWidgetInstance = CreateWidget<UObjectiveWidget>(PlayerController, ObjectiveWidgetClass);
	if (!ObjectiveWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create ObjectiveWidget."));
		return;
	}

	ObjectiveWidgetInstance->UpdateProgressText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentObjectiveNumber, TotalObjectiveNumber)));
	ObjectiveWidgetInstance->AddToViewport();
}

// Creates a new objective for the provided recipe and creates or updates the widget
void AObjectiveManager::ShowObjectiveForRecipe(FName RecipeResultItemID)
{
	// Extract data corresponding to RecipeResultItemID from CraftingData table
	if (!LookUpRecipeAndSetVariables(RecipeResultItemID))
	{
		return;
	}	

	ShowObjectiveProgress(CurrentObjectiveNumber, TotalObjectiveNumber);
}

// Removes the ObjectiveWidget if one exists
void AObjectiveManager::HideObjective()
{
	if (ObjectiveWidgetInstance)
	{
		ObjectiveWidgetInstance->RemoveFromParent();
		ObjectiveWidgetInstance = nullptr;
	}
}

// Increments the number of objectives currently fulfilled and updates the widget
void AObjectiveManager::IncrementProgress()
{
	if (!ObjectiveWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("No objective exists. Unable to update the progress. Call ShowObjective first to create the widget."));
		return;
	}
	
	CurrentObjectiveNumber++;
	
	ObjectiveWidgetInstance->UpdateProgressText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentObjectiveNumber, TotalObjectiveNumber)));
}

// Extracts the recipe corresponding to the given result item from CraftingData table and sets TotalObjectiveNumber, CurrentObjectiveNumber, CurrentRecipeIngredients and CurrentRecipeResultItemID
// Returns true in case of success and false if failing
bool AObjectiveManager::LookUpRecipeAndSetVariables(FName RecipeResultItemID)
{
	// Get all recipes from CraftingData table
	if (!CraftingData)
	{
		UE_LOG(LogTemp, Warning, TEXT("CraftingData table is null. Unable to look up recipe."));
	}
	TArray<FCraftingRecipe*> Recipes;
	CraftingData->GetAllRows<FCraftingRecipe>(TEXT("Crafting Context"), Recipes);
	
	// Find the recipe with the right ResultItemID
	for (FCraftingRecipe* Recipe : Recipes)
	{
		if (!Recipe)
		{
			continue;
		}
		
		// If the ResultItemId doesn't match, check next recipe
		if (!Recipe->ResultItemID.IsEqual(RecipeResultItemID))
		{
			continue;
		}
		
		// Correct recipe found -> Set Variables
		CurrentRecipeIngredients = Recipe->RequiredIngredients;
		TotalObjectiveNumber = CurrentRecipeIngredients.Num();
		CurrentObjectiveNumber = 0;
		CurrentRecipeResultItemID = Recipe->ResultItemID;
        		
		// Check if the objective is already fully fulfilled
		if (!Inventory)
		{
			UE_LOG(LogTemp, Warning, TEXT("Inventory is null. Unable to check the initial objective progress."));
			return false;
		}
		TArray<FInventorySlot> InventorySlots = Inventory->InventorySlots;
		for (FInventorySlot Slot : InventorySlots)
		{
			if (Slot.ItemID == NAME_None)
			{
				continue;
			}
				
			// The item to be crafted is already in the inventory
			if (Slot.ItemID.IsEqual(CurrentRecipeResultItemID))
			{
				UE_LOG(LogTemp, Warning, TEXT("Recipe with ResultItemID %s is already crafted. Unable to create objective."), *RecipeResultItemID.ToString());
				return false;
			}
		}
		
		// Check if the objective is already partly fulfilled
		for (FName Item: CurrentRecipeIngredients)
		{
			for (FInventorySlot Slot : InventorySlots)
			{
				if (Slot.ItemID == NAME_None)
				{
					continue;
				}
				
				// One of required items is already in the inventory
				if (Slot.ItemID.IsEqual(Item))
				{
					CurrentObjectiveNumber++;
				}
			}
		}
		
		return true;
	}
	
	// Recipe not found
	UE_LOG(LogTemp, Warning, TEXT("Recipe with ResultItemID %s not found. Unable to create objective."), *RecipeResultItemID.ToString());
	return false;
}

void AObjectiveManager::OnItemPickedUp(FName ItemID, bool bFirstPickupEver)
{
	// Check if the collected item is needed for the current objective
	for (FName Item: CurrentRecipeIngredients)
	{		
		if (Item.IsEqual(ItemID))
		{
			// The item counts towards the goal -> increment progress
			IncrementProgress();
		}
	}
}

void AObjectiveManager::OnItemCrafted(FName ItemID)
{
	// Check if the crafted item is the goal for the current objective
	if (ItemID.IsEqual(CurrentRecipeResultItemID))
	{
		// Objective completed
		HideObjective();
	}
}
