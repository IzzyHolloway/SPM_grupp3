// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "InputActionValue.h"
#include "CineCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "BoatFunctionality.generated.h"

// Forward declares

// Input
class UInputMappingContext;
class UInputAction;
class UInputComponent;

// Movement
class UFloatingPawnMovement;

// Enter & Exit
class UBoxComponent;
class ACharacterAimi;
class ADock;

// Wardrobe
class ACoatPickup;

UCLASS()
class SPM_GRUPP3_API ABoatFunctionality : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABoatFunctionality();
	
	// ---------------------------- INPUT ----------------------------
	
	// Handles Movement and Rotation Input
	UFUNCTION()
	void MoveRotate(const FInputActionValue& Value);
	
	// Handle Sprint Input
	UFUNCTION()
	void StartSprint(const FInputActionValue& Value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& Value);
	
	// Handles Mouse Input
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	
	// Handles Interaction Input
	UFUNCTION()
	void Interact(const FInputActionValue& Value);
	
	// ----------------------------- ENTER -----------------------------
	
	// Reacts to the OnComponentBeginOverlap event of the EnterTrigger (for the player to enter the boat) - calls EnableEnteringBoat()
	UFUNCTION()
	void OnEnterTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	// Reacts to the OnComponentEndOverlap event of the EnterTrigger (for the player to enter the boat) - calls DisableEnteringBoat()
	UFUNCTION()
	void OnEnterTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	// Communicates to the player character that entering the boat is possible now and hands over a reference to this boat
	UFUNCTION()
	void EnableEnteringBoat(ACharacterAimi* PlayerCharacter);
	
	// Communicates to the player character that it isn't possible anymore to enter the boat and removes the reference to this boat
	UFUNCTION()
	void DisableEnteringBoat(ACharacterAimi* PlayerCharacter);
	
	// Returns offset the character should have to the boat's coordinate center when it gets placed in the boat
	UFUNCTION()
	FVector GetCharacterPositionOffset() const;
	
	// ------------------------------- EXIT -------------------------------
	
	// Setter for DockInReach, supposed to be called by a dock if the boat is in close proximity and exiting the boat should be enabled
	UFUNCTION(BlueprintCallable)
	void SetDockInReach(ADock* Dock);
	
	// Setter for DockInReach, supposed to be called by a dock if the boat leaves the dock's trigger zone and exiting the boat should be disabled
	UFUNCTION(BlueprintCallable)
	void RemoveDockInReach();
	
	// --------------------------- PROGRESSION ---------------------------
	
	UFUNCTION(BlueprintCallable)
	bool CanPlayerEnterBoat() const;

	UFUNCTION(BlueprintCallable)
	void ShowCannotEnterBoatMessage() const;
	
	// ----------------------------- CAMERA -----------------------------

	UFUNCTION(BlueprintImplementableEvent)
	void SetCameraPositionWhenExiting(UCineCameraComponent* CameraComponent);

	UFUNCTION(BlueprintImplementableEvent)
	void FixCameraAfterRepossessingPlayer();
	
	// ------------------------------ SOUND ------------------------------

	UFUNCTION(BlueprintImplementableEvent)
	void BoatSoundStop();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	TObjectPtr<UBoxComponent> CollisionComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	// ---------------------------- INPUT ----------------------------
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	// Move Input Actions (including rotation)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> MoveRotateAction;
	
	// Sprint Input Actions (boost)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> SprintAction;
	
	// Look Input Actions (mouse)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> LookAction;
	
	// Interact Input Actions (mouse)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> InteractAction;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// --------------------------- MOVEMENT ---------------------------
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	TObjectPtr<UFloatingPawnMovement> MovementComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float SprintMovementSpeed = 2000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float RotationSpeed = 50.f;
	
	// ---------------------------- CAMERA ----------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TObjectPtr<UCineCameraComponent> Camera;
	
	// -------------------------- ENTER & EXIT --------------------------
	
	// Trigger box: If the character is inside, entering the boat is possible
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enter & Exit")
	TObjectPtr<UBoxComponent> EnterTrigger;
	
	// Offset the character should have to the boat's coordinate center when it gets placed in the boat
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enter & Exit")
	FVector CharacterPositionOffset = FVector(0.0f, 0.0f, 0.0f);
	
	// ------------------------------- UI -------------------------------
	
	// Widget class for "Press X / Press E to enter boat"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boat|UI")
	TSubclassOf<UUserWidget> InteractPromptWidgetClass;

	UPROPERTY()
	UUserWidget* InteractPromptWidget;
	
	virtual void PossessedBy(AController* NewController) override;

	void ShowInteractPrompt();
	void HideInteractPrompt();
	
	// --------------------------- PROGRESSION ---------------------------
	
	/*
	 * This part is needed to check if you can board the boat
	 * Example:
	 *  - You started a quest in Island 1
	 *  - You need to complete the quest
	 *  - The player decides to board onto the boat before completing the quest
	 *  - The player should NOT be able to board the boat before they complete a started quest
	 *  
	 */
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boat|Progression")
	bool bRequiresFlagToEnterBoat = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boat|Progression", meta = (EditCondition = "bRequiresFlagToEnterBoat"))
	FName RequiredFlagToEnterBoat = "LitLantern";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boat|Progression")
	FText CannotEnterBoatMessage = FText::FromString("I should help them first before leaving.");

private:	
	// ------------------------------- EXIT -------------------------------
	
	// If in reach of a pier, reference to the corresponding Pier, otherwise null
	TObjectPtr<ADock> DockInReach;
	
	void ExitBoat();
	
	// --------------------------- MOVEMENT ---------------------------
	
	// The speed the MovementComponent has when sprint is inactive
	float DefaultMovementSpeed;
	
	// --------------------------- WARDROBE ---------------------------
	
	// If in reach of a coat pickup, reference to the corresponding coat pickup, otherwise null
	ACoatPickup* CoatPickupInReach;
};
