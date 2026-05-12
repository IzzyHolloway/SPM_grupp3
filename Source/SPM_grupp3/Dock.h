// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dock.generated.h"

class ABoatFunctionality;
class UBoxComponent;

UCLASS()
class SPM_GRUPP3_API ADock : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADock();
	
	// Reacts to the OnComponentBeginOverlap event of the EnterTrigger (for the player to enter the boat) - calls EnableEnteringBoat()
	UFUNCTION()
	void OnExitBoatTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// Reacts to the OnComponentEndOverlap event of the EnterTrigger (for the player to enter the boat) - calls DisableEnteringBoat()
	UFUNCTION()
	void OnExitBoatTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	// Returns offset the character should have to the dock's coordinate center when it gets placed on the dock
	UFUNCTION()
	FVector GetCharacterPositionOffset() const;
	
	// Adding Flag when docking. Like ArriveIsland1 and so on...
	UFUNCTION()
	void ApplyDockingProgressionFlag();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction with boat")
	TObjectPtr<UBoxComponent> ExitBoatTrigger;
	
	// Offset the character should have to the dock's coordinate center when it gets placed on the dock
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enter & Exit")
	FVector CharacterPositionOffset = FVector(0.0f, 0.0f, 0.0f);
	
	
	/******** PROGRESSION ********/
	
	// Progression flag required before the boat can dock here.
	// Leave as None if this dock should always be usable.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName RequiredFlagToDock = NAME_None;

	// Message shown if the player tries to dock here before completing quest.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FString LockedDockMessage = TEXT("I cannot dock here yet.");
	
	// Progression flag added when the player successfully docks/exits here.
	// Leave as None if this dock should not add any flag.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName FlagToAddWhenDocking = NAME_None;
	
	
	/************** WIDGETS TEMPORARY *************/
	// Widget class for "Press X / Press E to enter boat"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boat|UI")
	TSubclassOf<UUserWidget> EnterDockPromptWidgetClass;

	UPROPERTY()
	UUserWidget* EnterDockPromptWidget;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void ShowEnterDockPrompt();
	void HideEnterDockPrompt();
	
private:
	void EnableExitingBoat(ABoatFunctionality* Boat);
	void DisableExitingBoat(ABoatFunctionality* Boat);
	
};
