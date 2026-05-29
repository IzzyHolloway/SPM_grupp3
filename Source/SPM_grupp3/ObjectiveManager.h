// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectiveManager.generated.h"

class UObjectiveWidget;
class UTextBlock;

UCLASS()
class SPM_GRUPP3_API AObjectiveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AObjectiveManager();
	
	UFUNCTION(BlueprintCallable)
	// Creates or updates an ObjectiveWidget with the provided content
	void ShowObjective(FText ObjectiveName, int Number, UTexture2D* Image);
	
	UFUNCTION(BlueprintCallable)
	// Removes the ObjectiveWidget if one exists
	void HideObjective();
	
	UFUNCTION(BlueprintCallable)
	// Increments the number of objectives currently fulfilled and updates the widget
	void IncrementProgress();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget")
	TSubclassOf<UUserWidget> ObjectiveWidgetClass;

	UPROPERTY()
	UObjectiveWidget* ObjectiveWidgetInstance;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
private:
	// Number of Objectives to fulfill
	int TotalObjectiveNumber = 1;

	int CurrentObjectiveNumber = 0;
};
