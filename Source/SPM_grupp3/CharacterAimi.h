#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CharacterAimi.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class AInteractableActor;

/*
 * Main player character used for movement, interaction
 * This is a prototype, the movment is not finalized
 * 
 * IMPORTANT: The Camera Spring Arm doesn't change angle correctly, meaning that the angle is very weird now. I need to fix this.
 */

UCLASS()
class SPM_GRUPP3_API ACharacterAimi : public ACharacter
{
	GENERATED_BODY()

public:
	ACharacterAimi();
	

	// Prototype item counter
	//void AddCollectedItem(int32 Amount = 1);
	//bool HasRequiredItems() const;
	
	//Function for locking or unlocking the charcter movements. Made to be callable in blueprints or other cpp files
	UFUNCTION(BlueprintCallable)
	void SetMovementLocked(bool bLock);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	// INPUT MAPPING AND ACTIONS
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UFUNCTION()
	void Move(const FInputActionValue& Value);

	UFUNCTION()
	void Look(const FInputActionValue& Value);

	UFUNCTION()
	void Interact(const FInputActionValue& Value);
	
	UFUNCTION()
	void StartJump();
	
	UFUNCTION()
	void StopJump();

	// SPRING ARM AND CAMERA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	// INTERACTION DISTANCES AND VALUES
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionForwardOffset = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float MaxInteractionDistance = 250.f;

	UPROPERTY()
	TObjectPtr<AInteractableActor> CurrentInteractable;

	void UpdateInteractableCandidate();
	void SetCurrentInteractable(AInteractableActor* NewInteractable);
	
	/******* WARNING: DEBUG KEYS! REMOVE LATER!!!! ********/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Debug")
	TObjectPtr<UInputAction> DebugSolveIsland1Action;
	
	UFUNCTION()
	void DebugSolveIsland1();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bEnableDebugKeys = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Debug")
	TObjectPtr<UInputAction> DebugCraftLanternAction;
	
	UFUNCTION()
	void DebugCraftLantern();
	
	/*
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Objectives")
	int32 CollectedItemCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	int32 RequiredItemCount = 2;
	
	*/
};