// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "BoatFunctionality.generated.h"

// Forward declares
// Input
class UInputMappingContext;
class UInputAction;
class UInputComponent;

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
	//
	UFUNCTION()
	void DisableEnteringBoat(ACharacterAimi* PlayerCharacter);

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
	
	// ---------------------------- CAMERA ----------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;
	
	// -------------------------- ENTER & EXIT --------------------------
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enter & Exit")
	TObjectPtr<UBoxComponent> EnterTrigger;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
