#include "CharacterAimi.h"
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

/* WARNING, THIS INCLUDE IS ONLY FOR DEBUGGING, REMOVE LATER!! */
#include "ProgressionManager.h"

ACharacterAimi::ACharacterAimi()
{
	PrimaryActorTick.bCanEverTick = true;

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

	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
}

void ACharacterAimi::BeginPlay()
{
	Super::BeginPlay();

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
}

void ACharacterAimi::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateInteractableCandidate();
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
		
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacterAimi::StartJump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacterAimi::StopJump);
		}
		
		if (DebugSolveIsland1Action)
		{
			EnhancedInput->BindAction(DebugSolveIsland1Action, ETriggerEvent::Started, this, &ACharacterAimi::DebugSolveIsland1);
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
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}


void ACharacterAimi::Interact(const FInputActionValue& Value)
{
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (DialogueManager && DialogueManager->IsDialogueActive())
	{
		DialogueManager->AdvanceDialogue();
		return;
	}

	if (CurrentInteractable)
	{
		CurrentInteractable->Interact();

		UE_LOG(LogTemp, Warning, TEXT("Interacted with: %s"), *CurrentInteractable->GetName());
	}
}



void ACharacterAimi::UpdateInteractableCandidate()
{
	if (!GetWorld())
	{
		return;
	}
	
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (DialogueManager && DialogueManager->IsDialogueActive())
	{
		SetCurrentInteractable(nullptr);
		return;
	}
	

	const FVector Forward = GetActorForwardVector();
	const FVector Center = GetActorLocation() + Forward * InteractionForwardOffset;

	TArray<AActor*> OverlappingActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
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
	
	/*
	// Drawing the interaction spehere around the player. Generous for cozy gameplay
	DrawDebugSphere(
	GetWorld(),
	Center,
	InteractionRadius,
	16,
	CurrentInteractable ? FColor::Green : FColor::Red,
	false,
	0.0f
);
*/
	
	SetCurrentInteractable(BestCandidate);
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

void ACharacterAimi::StartJump()
{
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

	UE_LOG(LogTemp, Warning, TEXT("DEBUG: Island1PuzzleSolved flag added"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("DEBUG: Island1Solved added"));
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