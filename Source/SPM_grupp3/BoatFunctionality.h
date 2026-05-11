// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "InputActionValue.h"
#include "CineCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "BoatFunctionality.generated.h"

class ADock;
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

UCLASS()
class SPM_GRUPP3_API ABoatFunctionality : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABoatFunctionality();
	
	// Handles Movement and Rotation Input
	UFUNCTION()
	void MoveRotate(const FInputActionValue& Value);
	
	// Handles Mouse Input
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	
	// Handles Interaction Input
	UFUNCTION()
	void Interact(const FInputActionValue& Value);
	
	// -------------------------- ENTER & EXIT --------------------------
	
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
	
	// WARNING: Replace AActor with the C++ pier class as soon as it exists!
	UFUNCTION(BlueprintCallable)
	void SetDockInReach(ADock* Dock);
	UFUNCTION(BlueprintCallable)
	void RemoveDockInReach();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	// ---------------------------- INPUT ----------------------------
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	// Move Input Actions (including rotation)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> MoveRotateAction;
	
	// Look Input Actions (mouse)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> LookAction;
	
	// Interact Input Actions (mouse)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> InteractAction;
	
	// --------------------------- MOVEMENT ---------------------------
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	TObjectPtr<UFloatingPawnMovement> MovementComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float RotationSpeed = 50.f;
	
	// ---------------------------- CAMERA ----------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCineCameraComponent> Camera;
	
	// -------------------------- ENTER & EXIT --------------------------
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enter & Exit")
	TObjectPtr<UBoxComponent> EnterTrigger;
	
	// Offset the character should have to the boat's coordinate center when it gets placed in the boat
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enter & Exit")
	FVector CharacterPositionOffset = FVector(0.0f, 0.0f, 110.0f);

private:	
	// If in reach of a pier, reference to the corresponding Pier, otherwise null
	TObjectPtr<ADock> DockInReach;
	
	void ExitBoat();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
