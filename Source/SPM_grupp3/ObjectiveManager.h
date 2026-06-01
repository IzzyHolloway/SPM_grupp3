// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectiveManager.generated.h"

struct FInventorySlot;
class UObjectiveWidget;
class UTextBlock;
class UInventoryComponent;

UCLASS()
class SPM_GRUPP3_API AObjectiveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AObjectiveManager();
	
	// Creates a new objective for the provided recipe and creates or updates the widget
	UFUNCTION(BlueprintCallable)
	void ShowObjectiveForRecipe(FName RecipeResultItemID);
	
	// Removes the ObjectiveWidget if one exists
	UFUNCTION(BlueprintCallable)
	void HideObjective();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Type of widget to be created
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget")
	TSubclassOf<UUserWidget> ObjectiveWidgetClass;
	
	// Currently showed objective widget
	UPROPERTY()
	UObjectiveWidget* ObjectiveWidgetInstance;
	
	// Table listing the crafting recipes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UDataTable> CraftingData;
	
	// Handles OnItemPickedUp event defined by inventory: Increments progress if the item picked up is needed for the objective
	UFUNCTION()
	void OnItemPickedUp(FName ItemID, bool bFirstPickupEver);
	
	// Handles OnItemCrafted event defined by inventory: Deletes the current objective if the item crafted completes the objective
	UFUNCTION()
	void OnItemCrafted(FName ItemID);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
private:
	// Inventory component attached to the player character
	UPROPERTY()
	UInventoryComponent* Inventory;
	
	// Extracts the recipe corresponding to the given result item from CraftingData table and sets TotalObjectiveNumber, CurrentObjectiveNumber, CurrentRecipeIngredients and CurrentRecipeResultItemID
	// Returns true in case of success and false if failing
	bool LookUpRecipeAndSetVariables(FName RecipeResultItemID);
	
	// Increments the number of objectives currently fulfilled and updates the widget
	void IncrementProgress();
	
	// -------------------------------- Current Objective Information --------------------------------
	
	// Number of items to collect for the current objective
	int TotalObjectiveNumber = 1;

	// Number of items collected for the current objective
	int CurrentObjectiveNumber = 0;
	
	// Items needed to craft the item corresponding to the current objective
	TArray<FName> CurrentRecipeIngredients;
	
	// Item to be crafted at the end of the current objective
	FName CurrentRecipeResultItemID;
	
};
