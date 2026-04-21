// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProgressionManager.generated.h"

/*
 * 
 * Story and gameplay progression state
 * 
 * - Store progression flags (FoundItemA, TalkedToGuideNPC)
 * - Store current objective text shown to player
 * 
 * Progression/States in this separate file makes it easier for different objects
 * (Like NPC, boat, pickups) to share the logic
 * 
 */

class UObjectiveWidgetBase;

UCLASS()
class SPM_GRUPP3_API AProgressionManager : public AActor
{
	GENERATED_BODY()

public:
	AProgressionManager();
	
	virtual void BeginPlay() override;

	// Adding a Progression Flag
	UFUNCTION(BlueprintCallable)
	void AddFlag(FName FlagName);

	// Removing a Progression Flag
	UFUNCTION(BlueprintCallable)
	void RemoveFlag(FName FlagName);

	// Does the flag exist in the current progression state?
	UFUNCTION(BlueprintCallable)
	bool HasFlag(FName FlagName) const;

	// clears all progression flags
	UFUNCTION(BlueprintCallable)
	void ClearAllFlags();
	
	// Set current objective text shown to player
	UFUNCTION(BlueprintCallable)
	void SetCurrentObjectiveText(const FText& NewObjectiveText);
	
	// Returns current obejctive text
	UFUNCTION(BlueprintCallable)
	FText GetCurrentObjectiveText() const;
	
	// Set current objective ID
	UFUNCTION(BlueprintCallable)
	void SetCurrentObjectiveID(FName NewObjectiveID);
	
	// Returns current objective ID
	UFUNCTION(BlueprintCallable)
	FName GetCurrentObjectiveID() const;

protected:
	// Progression flags shared by dialogue, pickups, boats etc...
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	TSet<FName> ProgressFlags;
	
	// Objective text for UI
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Objective")
	FText CurrentObjectiveText;
	
	// Objective identifier
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Objective")
	FName CurrentObjectiveID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	TSubclassOf<UObjectiveWidgetBase> ObjectiveWidgetClass;

	UPROPERTY()
	TObjectPtr<UObjectiveWidgetBase> ObjectiveWidgetInstance;
	
	void RefreshObjectiveWidget();
	
};