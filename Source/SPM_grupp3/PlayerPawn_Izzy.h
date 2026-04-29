#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "PlayerPawn_Izzy.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* VisualMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UCameraComponent* PlayerCamera;

    // --- INPUT ASSETS (Måste sättas i BP) ---
    UPROPERTY(EditAnywhere, Category = "Input")
    UInputMappingContext* InputMapping;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* JumpAction;

    // --- INSTÄLLNINGAR ---
    UPROPERTY(EditAnywhere, Category = "Movement")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    UPROPERTY(EditAnywhere, Category = "Movement")
    FVector CollisionExtent = FVector(50.f, 50.f, 50.f); // Storlek på boxen

    UPROPERTY(EditAnywhere, Category = "Movement")
    float Acceleration = 2500.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float JumpHeight = 1200.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float AirResistance = 0.15f; // Hur mycket fart vi tappar (0.1 = 10% kvar efter 1 sek)

    UPROPERTY(EditAnywhere, Category = "Movement")
    float SkinWidth = 2.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float GroundCheckDistance = 10.f;

private:
    void OnMove(const FInputActionValue& Value);
    void OnJump(const FInputActionValue& Value);
    
    void CollisionFunction(float DeltaTime);
    FVector CalculateNormalForce(FVector Force, FVector Normal) const;
    
    FVector CurrentInput = FVector::ZeroVector;
    FVector Velocity = FVector::ZeroVector;
};