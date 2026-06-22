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
class ABoatFunctionality;

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

	// ENTERING BOAT
	UFUNCTION()
	void SetBoatInReach(ABoatFunctionality* Boat);
	
	// ANIMATION
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	bool IsBoating = false;
	
	UFUNCTION()
	void RemoveBoatInReach();

	// Prototype item counter
	//void AddCollectedItem(int32 Amount = 1);
	//bool HasRequiredItems() const;
	
	//Function for locking or unlocking the charcter movements. Made to be callable in blueprints or other cpp files
	UFUNCTION(BlueprintCallable)
	void SetMovementLocked(bool bLock);

	// Enable / disable the per-tick "what am I looking at" interactable detection.
	// While disabled, no prompt widgets get created and CurrentInteractable is force-nulled.
	// Crafting / wardrobe call this with false on open and true on close, so the interact
	// prompt doesn't pop back up under the open UI.
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionDetectionEnabled(bool bEnabled);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Caught by World Settings' Kill Z: instead of the default (destroy the pawn), send the
	// player back to safety. A global backup for the BP_ResetZone water boxes, so a fall is
	// recoverable even where no box was placed. Set each level's Kill Z just under the water.
	virtual void FellOutOfWorld(const class UDamageType& DmgType) override;
	
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
	TObjectPtr<UInputAction> DialogueAdvanceAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UFUNCTION()
	void Move(const FInputActionValue& Value);

	UFUNCTION()
	void Look(const FInputActionValue& Value);

	UFUNCTION()
	void Interact(const FInputActionValue& Value);
	
	UFUNCTION()
	void AdvanceDialoguePressed(const FInputActionValue& Value);

	UFUNCTION()
	void AdvanceDialogueReleased(const FInputActionValue& Value);
	
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
	
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	int32 InteractionBlockCount = 0;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void PushInteractionBlock();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void PopInteractionBlock();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsInteractionBlocked() const;

	UPROPERTY()
	TObjectPtr<AInteractableActor> CurrentInteractable;

	// When false, Tick skips UpdateInteractableCandidate so prompts don't show up
	// behind an open UI (crafting, wardrobe, etc).
	bool bInteractionDetectionEnabled = true;

	void UpdateInteractableCandidate();
	void SetCurrentInteractable(AInteractableActor* NewInteractable);
	
	/******* WARNING: DEBUG KEYS! REMOVE LATER!!!! ********/
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Debug")
	TObjectPtr<UInputAction> DebugSolveIsland3Action;
	
	UFUNCTION()
	void DebugSolveIsland3();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Debug")
	TObjectPtr<UInputAction> DebugSolveIsland2Action;
	
	UFUNCTION()
	void DebugSolveIsland2();
	
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

	private:
		// ENTERING BOAT
	
		// If in reach of boat, reference to the corresponding BoatFunctionality, otherwise null.
		// UPROPERTY so the GC tracks it and clears it if the boat is ever destroyed (no dangling).
		UPROPERTY()
		TObjectPtr<ABoatFunctionality> BoatInReach;
		
		void EnterBoat();

		// ------------------------------ RESPAWN ------------------------------

		// Height added above the respawn point when recovering from a fall, so the player
		// drops in cleanly instead of spawning embedded in the ground.
		UPROPERTY(EditAnywhere, Category = "Respawn")
		float RespawnZOffset = 50.f;

		// Where the player started this level; fallback respawn target before any checkpoint.
		FVector SpawnLocation = FVector::ZeroVector;

		// The spot to send the player to after a fall: the BP_SaveLocation checkpoint variable
		// "LastSavedPosition" if it's been set, otherwise SpawnLocation.
		FVector GetRespawnLocation() const;
};