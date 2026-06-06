// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dock.generated.h"

class ABoatFunctionality;
class UBoxComponent;
class UPrimitiveComponent;
class UUserWidget;

UCLASS()
class SPM_GRUPP3_API ADock : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADock();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Returns offset the character should have to the dock's coordinate center when it gets placed on the dock
	UFUNCTION()
	FVector GetCharacterPositionOffset() const;
	
	// ------------- TRIGGER BOXES FOR DOCKING SPOTS ----------------
	
	// Reacts to the OnComponentBeginOverlap event of the right ExitBoatTrigger (for the player to enter the boat) - calls EnableEnteringBoat()
	UFUNCTION()
	void OnExitBoatTriggerRightBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// Reacts to the OnComponentEndOverlap event of the right ExitBoatTrigger (for the player to enter the boat) - calls DisableEnteringBoat()
	UFUNCTION()
	void OnExitBoatTriggerRightEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	// Reacts to the OnComponentBeginOverlap event of the right ExitBoatTrigger (for the player to enter the boat) - calls EnableEnteringBoat()
	UFUNCTION()
	void OnExitBoatTriggerLeftBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// Reacts to the OnComponentEndOverlap event of the right ExitBoatTrigger (for the player to enter the boat) - calls DisableEnteringBoat()
	UFUNCTION()
	void OnExitBoatTriggerLeftEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	// ------------------------- DOCKING SPOTS -------------------------
	
	// Returns the position the boat should have after docking
	UFUNCTION()
	FVector GetDockingSpotPosition() const;
	
	// Returns the rotation the boat should have after docking
	UFUNCTION()
	FRotator GetDockingSpotRotation() const;
	
	// ----------------------- PROGRESSION (AIMI) -----------------------
	
	// Adding Flag when docking. Like ArriveIsland1 and so on...
	UFUNCTION()
	void ApplyDockingProgressionFlag();
	
	// Called by the boat before allowing the player to board.
	UFUNCTION(BlueprintCallable)
	bool CanLeaveDock() const;

	UFUNCTION(BlueprintCallable)
	FText GetCannotLeaveDockMessage() const;
	
	// ----------------------- UI (ALICE OR IZZY) ------------------------
	
	void ShowEnterDockPrompt();
	void HideEnterDockPrompt();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction with boat")
	TObjectPtr<UBoxComponent> ExitBoatTriggerRight;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction with boat")
	TObjectPtr<UBoxComponent> ExitBoatTriggerLeft;
	
	// Offset the character should have to the dock's coordinate center when it gets placed on the dock
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enter & Exit")
	FVector CharacterPositionOffset = FVector(0.0f, 0.0f, 0.0f);
	
	// ------------------------- DOCKING SPOTS -------------------------
	
	// Boat position
	
	// Where the boat should dock when it comes from the right
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enter & Exit")
	FVector RightDockingSpotPosition = FVector(0.0f, 0.0f, 0.0f);
	
	// Where the boat should dock when it comes from the left
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enter & Exit")
	FVector LeftDockingSpotPosition = FVector(0.0f, 0.0f, 0.0f);
	
	// In which rotation the boat should dock when it comes from the right
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enter & Exit")
	FRotator RightDockingSpotRotation = FRotator(0.0f, 0.0f, 0.0f);
	
	// In which rotation the boat should dock when it comes from the left
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enter & Exit")
	FRotator LeftDockingSpotRotation = FRotator(0.0f, 0.0f, 0.0f);
	
	// ----------------------- PROGRESSION (AIMI) -----------------------
	
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
	
	// If true, the player cannot board the boat from this dock until this flag is active.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Progression|Leaving")
	bool bRequiresFlagToLeaveDock = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Progression|Leaving", meta = (EditCondition = "bRequiresFlagToLeaveDock"))
	FName RequiredFlagToLeaveDock = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression|Leaving")
	FText CannotLeaveDockMessage = FText::FromString(TEXT("I should finish helping here before leaving."));

	// ---------------------- UI (ALICE OR IZZY) ----------------------
	
	// Widget class for "Press X / Press E to enter boat"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boat|UI")
	TSubclassOf<UUserWidget> EnterDockPromptWidgetClass;

	UPROPERTY()
	UUserWidget* EnterDockPromptWidget;
	
private:
	void EnableExitingBoat(ABoatFunctionality* Boat);
	void DisableExitingBoat(ABoatFunctionality* Boat);
	
	// Position where the boat should dock (e.g. on the right of the dock)
	// Either RightDockingSpotPosition or LeftDockingSpotPosition
	FVector* CurrentDockingSpotPosition = nullptr;
	
	// Rotation in which the boat should dock (e.g. on the right of the dock)
	// Either RightDockingSpotRotation or LeftDockingSpotRotation
	FRotator* CurrentDockingSpotRotation = nullptr;
	
	// ----------------------- PROGRESSION (AIMI) -----------------------
	
	bool IsExitingBoatAllowed(ABoatFunctionality* Boat);
};
