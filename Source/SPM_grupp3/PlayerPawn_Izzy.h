// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawn_Izzy.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class SPM_GRUPP3_API APlayerPawn_Izzy : public APawn
{
	GENERATED_BODY()

public:
	APlayerPawn_Izzy();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	// Components
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* VisualMesh;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* PlayerCamera;

	// Input
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* InputMapping;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	// Movement tuning
	UPROPERTY(EditAnywhere, Category = "Movement")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float Acceleration = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float JumpStrength = 900.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float AirResistance = 0.3f; // 0-1, högre = mer motstånd

	UPROPERTY(EditAnywhere, Category = "Movement")
	float StaticFriction = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float KineticFriction = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float SkinWidth = 2.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float GroundCheckDistance = 5.f;

private:
	// Input callbacks
	void OnMove(const FInputActionValue& Value);
	void OnJump(const FInputActionValue& Value);

	// Physics
	void ResolveCollisions(float DeltaTime);
	FVector CalculateNormalForce(FVector Force, FVector Normal) const;
	void ApplyFriction(float NormalForceMagnitude);
	void Depenetrate();
	bool IsGrounded() const;

	// State
	FVector CurrentInput = FVector::ZeroVector;
	FVector Velocity = FVector::ZeroVector;
};