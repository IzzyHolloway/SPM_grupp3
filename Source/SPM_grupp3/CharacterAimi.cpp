#include "CharacterAimi.h"
#include "DrawDebugHelpers.h"
#include "InteractableActor.h"
#include "DialogueManager.h"
#include "Kismet/GameplayStatics.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/UnrealType.h"
#include "Engine/World.h"
#include "Engine/LevelScriptActor.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

/* WARNING, THIS INCLUDE IS ONLY FOR DEBUGGING, REMOVE LATER!! */
#include "AIController.h"
#include "ProgressionManager.h"

#include "BoatFunctionality.h"
#include "BehaviorTree/BlackboardComponent.h"

ACharacterAimi::ACharacterAimi()
{
	PrimaryActorTick.bCanEverTick = true;
	
	InteractionBlockCount = 0;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300.f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	GetCharacterMovement()->RotationRate = FRotator(0.f, 300.f, 0.f);
	
	/* Movement Polishing. Can be changed later */
	GetCharacterMovement()->MaxWalkSpeed = 320.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 900.f;
	GetCharacterMovement()->GroundFriction = 6.f;
	
	GetCharacterMovement()->JumpZVelocity = 420.f;
	GetCharacterMovement()->AirControl = 0.3f;
}

void ACharacterAimi::BeginPlay()
{
	Super::BeginPlay();

	// Remember where we spawned as a last-resort respawn point (used until the player reaches
	// their first BP_SaveLocation checkpoint).
	SpawnLocation = GetActorLocation();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}

	// Hide the interact prompt for a moment so it doesn't flash over the level-load / loading
	// screen. The timer turns detection back on (unless a cutscene is still suppressing it).
	SetInteractionDetectionEnabled(false);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(LevelLoadSuppressTimer, this,
			&ACharacterAimi::EndLevelLoadInteractionSuppress, LevelLoadInteractionSuppressSeconds, false);
	}
}

void ACharacterAimi::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateInteractableCandidate();
	UpdateCutsceneLock();
}

void ACharacterAimi::UpdateCutsceneLock()
{
	// The Level Blueprint flips its own bool "bIsInCutscene" true while a cutscene video plays
	// and false when it ends or is skipped. Mirror that onto the player's movement -- with no
	// Blueprint wiring -- by reading the flag via reflection and acting only on the edge.
	UWorld* World = GetWorld();
	ALevelScriptActor* LevelScript = World ? World->GetLevelScriptActor() : nullptr;

	// Re-resolve the (cached) flag property only when the level / its script actor changes.
	if (LevelScript != CachedLevelScript.Get())
	{
		CachedLevelScript = LevelScript;
		CachedCutsceneFlagProp = LevelScript
			? CastField<FBoolProperty>(LevelScript->GetClass()->FindPropertyByName(TEXT("bIsInCutscene")))
			: nullptr;
	}

	const bool bInCutscene = (LevelScript && CachedCutsceneFlagProp)
		? CachedCutsceneFlagProp->GetPropertyValue_InContainer(LevelScript)
		: false;

	// --- Enter / leave the cutscene state on the edge ---
	if (bInCutscene && !bCutsceneLockActive)
	{
		// Cutscene started: block input, camera and the interact prompt right away, but DON'T
		// freeze movement yet -- let gravity carry the player down so they land naturally instead
		// of freezing mid-air. The full freeze happens below, once they're on the ground.
		bCutsceneLockActive = true;
		bCutsceneMovementFrozen = false;
		SetInteractionDetectionEnabled(false);
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->SetIgnoreMoveInput(true);
			PC->SetIgnoreLookInput(true);
		}
	}
	else if (!bInCutscene && bCutsceneLockActive)
	{
		// Cutscene ended or was skipped: restore everything.
		bCutsceneLockActive = false;
		if (bCutsceneMovementFrozen)
		{
			SetMovementLocked(false); // MOVE_None -> Walking
			bCutsceneMovementFrozen = false;
		}
		SetInteractionDetectionEnabled(true);
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->ResetIgnoreMoveInput();
			PC->ResetIgnoreLookInput();
		}
	}

	// While the cutscene runs, fully freeze movement (MOVE_None, so no nudging or jumping) the
	// moment the player is back on the ground. Before that, gravity does its job.
	if (bCutsceneLockActive && !bCutsceneMovementFrozen)
	{
		if (const UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			if (Movement->IsMovingOnGround())
			{
				SetMovementLocked(true);
				bCutsceneMovementFrozen = true;
			}
		}
	}
}

void ACharacterAimi::EndLevelLoadInteractionSuppress()
{
	// Don't re-enable detection if a cutscene (or another system) is still holding it off; that
	// system turns it back on when it's done.
	if (!bCutsceneLockActive)
	{
		SetInteractionDetectionEnabled(true);
	}
}

void ACharacterAimi::FellOutOfWorld(const UDamageType& DmgType)
{
	// Default behaviour destroys the pawn. Instead, recover the player. This is the global
	// backup for the BP_ResetZone water boxes: World Settings' Kill Z (set just under the
	// water surface) catches a fall anywhere, even where no trigger box was placed.

	if (IsBoating)
	{
		// Attached to the boat, which floats above the water -- don't yank the player out of
		// it. (Returning without calling Super also skips the default "destroy".)
		return;
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}

	const FVector Target = GetRespawnLocation() + FVector(0.f, 0.f, RespawnZOffset);
	SetActorLocation(Target, false, nullptr, ETeleportType::TeleportPhysics);
}

FVector ACharacterAimi::GetRespawnLocation() const
{
	// Prefer the checkpoint that BP_SaveLocation writes (the Blueprint variable
	// "LastSavedPosition"), so this backup lands the player exactly where the trigger-box
	// system would. It lives on the Blueprint, so read it via reflection.
	if (const FStructProperty* SavedProp = CastField<FStructProperty>(GetClass()->FindPropertyByName(TEXT("LastSavedPosition"))))
	{
		if (SavedProp->Struct == TBaseStructure<FVector>::Get())
		{
			const FVector Saved = *SavedProp->ContainerPtrToValuePtr<FVector>(this);
			if (!Saved.IsNearlyZero())
			{
				return Saved;
			}
		}
	}

	// No checkpoint reached yet (still at origin): fall back to the level spawn point.
	return SpawnLocation;
}

void ACharacterAimi::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACharacterAimi::Move);
		}

		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACharacterAimi::Look);
		}
		
		if (InteractAction)
		{
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ACharacterAimi::Interact);
		}
		
		if (DialogueAdvanceAction)
		{
			EnhancedInput->BindAction(DialogueAdvanceAction, ETriggerEvent::Started, this, &ACharacterAimi::AdvanceDialoguePressed);
			EnhancedInput->BindAction(DialogueAdvanceAction, ETriggerEvent::Completed, this, &ACharacterAimi::AdvanceDialogueReleased);
		}
		
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacterAimi::StartJump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacterAimi::StopJump);
		}
		
		if (DebugSolveIsland3Action)
		{
			EnhancedInput->BindAction(DebugSolveIsland3Action, ETriggerEvent::Started, this, &ACharacterAimi::DebugSolveIsland3);
		}
		
		if (DebugSolveIsland2Action)
		{
			EnhancedInput->BindAction(DebugSolveIsland2Action, ETriggerEvent::Started, this, &ACharacterAimi::DebugSolveIsland2);
		}
		
		if (DebugSolveIsland1Action)
		{
			EnhancedInput->BindAction(DebugSolveIsland1Action, ETriggerEvent::Started, this, &ACharacterAimi::DebugSolveIsland1);
		}
		
		if (DebugCraftLanternAction)
		{
			EnhancedInput->BindAction(DebugCraftLanternAction, ETriggerEvent::Started, this, &ACharacterAimi::DebugCraftLantern);
		}
		
		/*
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacterAimi::Jump);
		}
		*/
	}
}

/*
void ACharacterAimi::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}
*/

void ACharacterAimi::Move(const FInputActionValue& Value)
{
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
	UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
);

	if (DialogueManager && DialogueManager->IsDialogueActive())
	{
		return;
	}
	
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (MovementVector.IsNearlyZero() || !Controller)
	{
		return;
	}
	
	// Use camera/controller yaw to make movement relative to the current view.
	// This makes W feel like "forward on screen" in a normal third-person test camera.
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ACharacterAimi::Look(const FInputActionValue& Value)
{
	// Making sure character can't look around while in dialogue. Remove if not needed
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
	UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
);

	if (DialogueManager && DialogueManager->IsDialogueActive())
	{
		return;
	}
	
	
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}


void ACharacterAimi::Interact(const FInputActionValue& Value)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (PC && PC->bShowMouseCursor)
	{
		return;
	}
	
	if (IsInteractionBlocked())
	{
		return;
	}
	
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (DialogueManager && DialogueManager->IsDialogueOrMessageVisible())
	{
		SetCurrentInteractable(nullptr);
		return;
	}

	if (CurrentInteractable)
	{
		// Capture the name up front: Interact() can destroy or clear the interactable (a pickup
		// being consumed, a boat/dock handoff, etc.), which leaves CurrentInteractable dangling.
		// Reading ->GetName() after the call then dereferences a dead pointer -- this was the
		// recurring Interact() crash (CharacterAimi.cpp:243 in the crash dumps).
		const FString InteractableName = CurrentInteractable->GetName();
		CurrentInteractable->Interact();

		UE_LOG(LogTemp, Warning, TEXT("Interacted with: %s"), *InteractableName);
	}

	// ENTERING BOAT
	
	// Enter the boat if close enough
	if (BoatInReach != nullptr)
	{
		EnterBoat();
	}
}



void ACharacterAimi::UpdateInteractableCandidate()
{
	if (!GetWorld())
	{
		return;
	}

	// Suppressed by an open UI (crafting bench, wardrobe, etc). Clear current so the
	// prompt disappears and doesn't come back via Tick while the UI is up.
	if (!bInteractionDetectionEnabled)
	{
		SetCurrentInteractable(nullptr);
		return;
	}

	// While the player is movement-locked (MOVE_None) -- a cutscene, the gate cutscene video,
	// or a level transition where SetMovementLocked(true) was called -- hide the interact
	// prompt: if you can't move, you can't interact. Keeps the prompt off the gate video and
	// the black loading screen with no Blueprint wiring.
	if (const UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		if (Movement->MovementMode == MOVE_None)
		{
			SetCurrentInteractable(nullptr);
			return;
		}
	}

	// Hide the prompt while the gate cutscene video (WBP_Cutscene) is on screen. The player can
	// still walk during that video (it isn't movement-locked), so the MOVE_None rule above misses
	// it -- detect the widget directly instead.
	if (IsGateCutsceneWidgetOnScreen())
	{
		SetCurrentInteractable(nullptr);
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (PC && PC->bShowMouseCursor)
	{
		SetCurrentInteractable(nullptr);
		return;
	}

	if (IsInteractionBlocked())
	{
		SetCurrentInteractable(nullptr);
		return;
	}
	
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	/*
	if (DialogueManager && DialogueManager->IsDialogueActive())
	{
		SetCurrentInteractable(nullptr);
		return;
	}
	*/
	
	if (DialogueManager && DialogueManager->IsDialogueOrMessageVisible())
	{
		SetCurrentInteractable(nullptr);
		return;
	}
	

	const FVector Forward = GetActorForwardVector();
	const FVector Center = GetActorLocation() + Forward * InteractionForwardOffset;

	TArray<AActor*> OverlappingActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	const bool bFoundAny = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Center,
		InteractionRadius,
		ObjectTypes,
		AInteractableActor::StaticClass(),
		ActorsToIgnore,
		OverlappingActors
	);

	AInteractableActor* BestCandidate = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();

	if (bFoundAny)
	{
		for (AActor* Actor : OverlappingActors)
		{
			if (!IsValid(Actor))
			{
				continue;
			}

			AInteractableActor* Candidate = Cast<AInteractableActor>(Actor);
			if (!IsValid(Candidate))
			{
				continue;
			}

			const FVector ToCandidate = Candidate->GetActorLocation() - GetActorLocation();
			const float DistanceSq = ToCandidate.SizeSquared();

			if (DistanceSq > FMath::Square(MaxInteractionDistance))
			{
				continue;
			}

			const FVector ToCandidateDir = ToCandidate.GetSafeNormal();
			const float Dot = FVector::DotProduct(GetActorForwardVector(), ToCandidateDir);

			if (Dot < 0.3f)
			{
				continue;
			}

			if (DistanceSq < BestDistanceSq)
			{
				BestDistanceSq = DistanceSq;
				BestCandidate = Candidate;
			}
		}
	}
	

	
	SetCurrentInteractable(BestCandidate);
}

bool ACharacterAimi::IsGateCutsceneWidgetOnScreen()
{
	// Resolve the WBP_Cutscene class once (the widget BP_Gate shows for the gate cutscene video).
	if (!CachedGateCutsceneWidgetClass)
	{
		CachedGateCutsceneWidgetClass = LoadClass<UUserWidget>(nullptr,
			TEXT("/Game/Blueprints/WBP/CutScenes/WBP_Cutscene.WBP_Cutscene_C"));
		if (!CachedGateCutsceneWidgetClass)
		{
			return false;
		}
	}

	TArray<UUserWidget*> Found;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), Found, CachedGateCutsceneWidgetClass, false);
	for (const UUserWidget* Widget : Found)
	{
		if (Widget && Widget->IsInViewport())
		{
			return true;
		}
	}
	return false;
}

void ACharacterAimi::SetCurrentInteractable(AInteractableActor* NewInteractable)
{
	if (CurrentInteractable == NewInteractable)
	{
		return;
	}

	if (IsValid(CurrentInteractable))
	{
		CurrentInteractable->SetPromptVisible(false);
	}

	CurrentInteractable = NewInteractable;

	if (IsValid(CurrentInteractable))
	{
		CurrentInteractable->SetPromptVisible(true);
	}
}

void ACharacterAimi::PushInteractionBlock()
{
	InteractionBlockCount++;

	// Hide current prompt immediately
	SetCurrentInteractable(nullptr);
}

void ACharacterAimi::PopInteractionBlock()
{
	InteractionBlockCount = FMath::Max(0, InteractionBlockCount - 1);
}

bool ACharacterAimi::IsInteractionBlocked() const
{
	return InteractionBlockCount > 0;
}

void ACharacterAimi::AdvanceDialoguePressed(const FInputActionValue& Value)
{
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (DialogueManager && DialogueManager->IsDialogueActive())
	{
		DialogueManager->StartDialogueAdvanceHold();
	}
}

void ACharacterAimi::AdvanceDialogueReleased(const FInputActionValue& Value)
{
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (DialogueManager && DialogueManager->IsDialogueActive())
	{
		DialogueManager->FinishDialogueAdvanceHold();
	}
}

void ACharacterAimi::StartJump()
{
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (DialogueManager && DialogueManager->IsDialogueActive())
	{
		return;
	}

	Jump();
}

void ACharacterAimi::StopJump()
{
	StopJumping();
}

/******* WARNING: DEBUG FUNCTIONS! REMOVE LATER!!!! ********/
void ACharacterAimi::DebugSolveIsland1()
{
	if (!bEnableDebugKeys)
	{
		return;
	}

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		return;
	}

	ProgressionManager->AddFlag("Island1PuzzleSolved");

	UE_LOG(LogTemp, Warning, TEXT("DEBUG: Island2PuzzleSolved flag added"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("DEBUG: Island1Solved added"));
	}
	

}

void ACharacterAimi::DebugSolveIsland2()
{
	if (!bEnableDebugKeys)
	{
		return;
	}

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		return;
	}

	ProgressionManager->AddFlag("GramophonePlayed");

	UE_LOG(LogTemp, Warning, TEXT("DEBUG: GramophonePlayed flag added"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("DEBUG: Island1Solved added"));
	}
}

void ACharacterAimi::DebugSolveIsland3()
{
	if (!bEnableDebugKeys)
	{
		return;
	}

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		return;
	}

	ProgressionManager->AddFlag("Island3PuzzleSolved");

	UE_LOG(LogTemp, Warning, TEXT("DEBUG: Island3PuzzleSolved flag added"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("DEBUG: Island3Solved added"));
	}
}

void ACharacterAimi::DebugCraftLantern()
{
	
	if (!bEnableDebugKeys)
	{
		return;
	}

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		return;
	}
	
	ProgressionManager->AddFlag("LitLantern");
	
	UE_LOG(LogTemp, Warning, TEXT("DEBUG: LitLantern"));
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("DEBUG: LitLantern added"));
	}
}

void ACharacterAimi::SetInteractionDetectionEnabled(bool bEnabled)
{
	bInteractionDetectionEnabled = bEnabled;

	// On disable, immediately drop the current prompt so it disappears the same frame the UI opens.
	if (!bEnabled)
	{
		SetCurrentInteractable(nullptr);
	}
}

//Lock or unlock the movement of the charaacter. Do nothing if we do not have CharcterMovement.
void ACharacterAimi::SetMovementLocked(bool bLock)
{
	if (!GetCharacterMovement())
	{
		return;
	}

	if (bLock)
	{
		GetCharacterMovement()->DisableMovement();
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}


/*
void ACharacterAimi::AddCollectedItem(int32 Amount)
{
	CollectedItemCount += Amount;

	UE_LOG(LogTemp, Warning, TEXT("Collected items: %d / %d"), CollectedItemCount, RequiredItemCount);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.f,
			FColor::Yellow,
			FString::Printf(TEXT("Collected items: %d / %d"), CollectedItemCount, RequiredItemCount)
		);
	}
}

// This one is hardcoded. I am going to remove after testing and updating.
bool ACharacterAimi::HasRequiredItems() const
{
	return CollectedItemCount >= RequiredItemCount;
}
*/

// ENTERING BOAT
void ACharacterAimi::EnterBoat()
{
	// Double check that we're in reach of a boat
	if (BoatInReach == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnterBoat() was called without a boat in reach. This shouldn't be happening!"));
		return;
	}

	// Save the boat in reach in case the character leaves its trigger zone while being moved onto the boat
	ABoatFunctionality* CurrentBoatInReach = BoatInReach;
	
	if (!CurrentBoatInReach->CanPlayerEnterBoat())
	{
		CurrentBoatInReach->ShowCannotEnterBoatMessage();
		return;
	}
	
	// Disable movement
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->DisableMovement();
	}
	
	// Lock the character's animations while in the boat
	IsBoating = true;
	
	// Attach character to the boat so it moves with the boat
	AttachToActor(CurrentBoatInReach, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true), EName::None);
	
	// Move character to right offset relative to the boat (so it sits "on" the boat and not "in" it)
	SetActorRelativeLocation(CurrentBoatInReach->GetCharacterPositionOffset());
	
	// Possess the boat (guard the controller -- never dereference a null GetController()).
	if (AController* OwningController = GetController())
	{
		OwningController->Possess(CurrentBoatInReach);
	}

}

void ACharacterAimi::SetBoatInReach(ABoatFunctionality* Boat)
{
	BoatInReach = Boat;
}

void ACharacterAimi::RemoveBoatInReach()
{
	BoatInReach = nullptr;
}