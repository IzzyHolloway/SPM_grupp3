// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveManager.h"

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
	
}

// Called every frame
void AObjectiveManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Creates or updates an ObjectiveWidget with the provided content
void AObjectiveManager::ShowObjective(FText ObjectiveDescription, int Number, UTexture2D* Image)
{
	// If the Objective Widget already exists, update it
	if (ObjectiveWidgetInstance)
	{
		ObjectiveWidgetInstance->UpdateObjectiveDescriptionText(ObjectiveDescription);
		ObjectiveWidgetInstance->UpdateProgressText(FText::FromString(FString::Printf(TEXT("0/%d"), Number)));
		ObjectiveWidgetInstance->UpdateImage(Image);
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
	
	// Set the content
	ObjectiveWidgetInstance->UpdateObjectiveDescriptionText(ObjectiveDescription);
	ObjectiveWidgetInstance->UpdateProgressText(FText::FromString(FString::Printf(TEXT("0/%d"), Number)));
	ObjectiveWidgetInstance->UpdateImage(Image);

	// Add the widget to the viewport
	if (ObjectiveWidgetInstance)
	{
		ObjectiveWidgetInstance->AddToViewport();
	}
	
	// Save the number of objectives to fulfill for later
	TotalObjectiveNumber = Number;
	CurrentObjectiveNumber = 0;
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
	
	ObjectiveWidgetInstance->UpdateProgressText(FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentObjectiveNumber, TotalObjectiveNumber)));
}
