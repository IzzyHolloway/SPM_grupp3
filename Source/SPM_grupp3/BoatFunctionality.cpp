// Fill out your copyright notice in the Description page of Project Settings.


#include "BoatFunctionality.h"

#include "AIController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"

#include "ProgressionManager.h"
#include "DialogueManager.h"

#include "CharacterAimi.h"
#include "Dock.h"

#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/MovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "BehaviorTree/BlackboardComponent.h"

// Sets default values
ABoatFunctionality::ABoatFunctionality()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	// Not needed in this script, but in Blueprint that inherits from the script
	PrimaryActorTick.bCanEverTick = true;
	
	// ---------------------------------- COLLISION ----------------------------------
	
	// FloatingPawnMovement needs a collision component as root
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	CollisionComponent->SetWorldRotation(FRotator(0.0, 90.0, 0.0));
	RootComponent = CollisionComponent;
	
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_Pawn);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	
	// ------------------------------------ MESH ------------------------------------
    
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	MeshComponent->SetupAttachment(RootComponent);
	
	// ----------------------------------- CAMERA -----------------------------------

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->SetRelativeLocation(FVector(-120, 0, 200));
	SpringArm->SetRelativeRotation(FRotator(0, -5, 0));
	SpringArm->TargetArmLength = 600.f;
	SpringArm->bUsePawnControlRotation = true;
	
	Camera = CreateDefaultSubobject<UCineCameraComponent>(TEXT("FollowCamera"));
	Camera->SetFilmbackPresetByName(TEXT("16:9 DSLR"));
	Camera->SetCurrentFocalLength(18);
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	
	// ---------------------------------- MOVEMENT ----------------------------------
	
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = CollisionComponent;
	
	// -------------------------------- ENTER & EXIT --------------------------------
	
	EnterTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EnterTrigger"));
	EnterTrigger->SetupAttachment(MeshComponent);
	
	// Subscribe OnEnterTriggerBeginOverlap function to the OnComponentBeginOverlap event of the enter trigger box
	EnterTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABoatFunctionality::OnEnterTriggerBeginOverlap);
	
	// Subscribe OnEnterTriggerEndOverlap function to the OnComponentEndOverlap event of the enter trigger box
	EnterTrigger->OnComponentEndOverlap.AddDynamic(this, &ABoatFunctionality::OnEnterTriggerEndOverlap);

}

// Called when the game starts or when spawned
void ABoatFunctionality::BeginPlay()
{
	Super::BeginPlay();
	
	// ---------------------------------- INPUT ----------------------------------
	
	check(GEngine != nullptr);
	
	// Get the player controller for this Pawn
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
	
	// -------------------------------- MOVEMENT ---------------------------------
	
	// Save default movement speed for resetting after sprint
	DefaultMovementSpeed = MovementComponent->GetMaxSpeed();
	
}

// Called every frame
void ABoatFunctionality::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// ------------------------------------------------------------------ INPUT ------------------------------------------------------------------

// Called to bind functionality to input
void ABoatFunctionality::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind Movement and Rotation Actions
		EnhancedInputComponent->BindAction(MoveRotateAction, ETriggerEvent::Triggered, this, &ABoatFunctionality::MoveRotate);
		
		// Bind Sprint Actions
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ABoatFunctionality::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABoatFunctionality::StopSprint);
		
		// Bind Look Actions
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABoatFunctionality::Look);
		
		// Bind Interact Actions
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ABoatFunctionality::Interact);
	}
}

// ----------------------------------------------------------------- MOVEMENT -----------------------------------------------------------------

void ABoatFunctionality::MoveRotate(const FInputActionValue& Value)
{
	// 2D Vector of movement values returned from the input action
	const FVector2D MovementValue = Value.Get<FVector2D>();
	
	// Check if the controller possessing this Actor is valid
	if (Controller)
	{
		// Apply rotation input
		AddActorWorldRotation(FRotator(0.f, MovementValue.X * RotationSpeed * GetWorld()->GetDeltaSeconds(), 0.f));
		
		// Apply forward and back movement
		AddMovementInput(GetActorForwardVector(), MovementValue.Y);
	}
}

void ABoatFunctionality::StartSprint(const FInputActionValue& Value)
{
	// Make movement faster
	MovementComponent->MaxSpeed = SprintMovementSpeed;
}

void ABoatFunctionality::StopSprint(const FInputActionValue& Value)
{	
	// Reset movement speed
	MovementComponent->MaxSpeed = DefaultMovementSpeed;
}

// ------------------------------------------------------------------ CAMERA ------------------------------------------------------------------

void ABoatFunctionality::Look(const FInputActionValue& Value)
{
	// 2D Vector of look values returned from the input action
	const FVector2D LookValue = Value.Get<FVector2D>();

	// Check if the controller possessing this Actor is valid
	if (Controller)
	{
		// Apply camera input
		AddControllerYawInput(LookValue.X);
		AddControllerPitchInput(LookValue.Y);
	}
}

// ---------------------------------------------------------------- INTERACTION -----------------------------------------------------------------

void ABoatFunctionality::Interact(const FInputActionValue& Value)
{
	HideEnterBoatPrompt();

	if (DockInReach != nullptr)
	{
		if (DockInReach)
		{
			DockInReach->HideEnterDockPrompt();
		}

		ExitBoat();
	}
}

// ------------------------------------------------------------------ ENTER ------------------------------------------------------------------

// Reacts to the OnComponentBeginOverlap event of the EnterTrigger (for the player to enter the boat) - calls EnableEnteringBoat()
void ABoatFunctionality::OnEnterTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	
	// Check if the overlapping object is the player character
	if (ACharacterAimi* PlayerCharacter = Cast<ACharacterAimi>(OtherActor))
	{
		// Enable entering the boat for the player
		EnableEnteringBoat(PlayerCharacter);
	}
}

// Communicates to the player character that entering the boat is possible now and hands over a reference to this boat
void ABoatFunctionality::EnableEnteringBoat(ACharacterAimi* PlayerCharacter)
{
	if (!PlayerCharacter)
	{
		return;
	}

	// Hand over a reference to myself to the player character to enable it to enter the boat
	PlayerCharacter->SetBoatInReach(this);

	// ------------------------------ UI ------------------------------
	
	if (DockInReach)
	{
		DockInReach->HideEnterDockPrompt();
	}

	// Show "Press X/E" UI
	ShowEnterBoatPrompt();
}

void ABoatFunctionality::OnEnterTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if the overlapping object is the player character
	if (ACharacterAimi* PlayerCharacter = Cast<ACharacterAimi>(OtherActor))
	{
		// Disable entering the boat for the player
		DisableEnteringBoat(PlayerCharacter);
	}
}

// Communicates to the player character that it isn't possible anymore to enter the boat and removes the reference to this boat
void ABoatFunctionality::DisableEnteringBoat(ACharacterAimi* PlayerCharacter)
{	
	// Remove the reference to myself in the player character to disable entering the boat
	PlayerCharacter->RemoveBoatInReach();
	
	// ------------------------------ UI ------------------------------
	
	HideEnterBoatPrompt();
}

// Returns offset the character should have to the boat's coordinate center when it gets placed in the boat
FVector ABoatFunctionality::GetCharacterPositionOffset() const
{
	return CharacterPositionOffset;
}

// ------------------------------------------------------------------ EXIT ------------------------------------------------------------------

void ABoatFunctionality::ExitBoat()
{
	// ------------------------------ UI ------------------------------
	
	HideEnterBoatPrompt();
	
	if (DockInReach)
	{
		DockInReach->HideEnterDockPrompt();
	}
	
	// ----------------------------------------------------------------
	
	// Double check that we're in reach of a pier
	if (DockInReach == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExitBoat() was called without a boat in reach. This shouldn't be happening!"));
		return;
	}
	
	// Safe the pier in case we move out of reach during the process
	TObjectPtr<ADock> CurrentDockInReach = DockInReach;

	// Set camera position
	SetCameraPositionWhenExiting(Camera);
	
	// ---------------------------- WATER AI ----------------------------
	/*
	//This gets an array with all the actors that has the tag WaterAI on it.
	TArray<AActor*> AICharacters;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("WaterAI"), AICharacters);
	
	//If we found an actor we will get its controller. 
	if (AICharacters.Num() > 0)
	{
		AAIController* AIController = Cast<AAIController>(Cast<APawn>(AICharacters[0])->GetController());
		if (AIController)
		{
			AIController->GetBlackboardComponent()->SetValueAsBool("IsInBoat", false);
		}
	}
	*/
	// ------------------------------------------------------------------
	
	// Move boat to docking spot
	SetActorLocation(CurrentDockInReach->GetDockingSpotPosition());
	SetActorRotation(CurrentDockInReach->GetDockingSpotRotation());
	
	// ExitAnimation
	// TODO
	
	// Find the player character among the children
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	for (AActor* AttachedActor : AttachedActors)
	{
		if (ACharacterAimi* PlayerCharacter = Cast<ACharacterAimi>(AttachedActor))
		{
			// Enable animation changes again
			PlayerCharacter->IsBoating = false;
			
			// Detach player character
			PlayerCharacter->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			
			// Move player character on top of the pier
			PlayerCharacter->SetActorLocation(CurrentDockInReach->GetActorLocation() + CurrentDockInReach->GetCharacterPositionOffset());
			
			// Hide dock prompt because we are exiting the boat now
			CurrentDockInReach->HideEnterDockPrompt();
			
			// Add the dock's arrival/progression flag, for example ArrivedIsland1, ArrivedIsland2, etc.
			CurrentDockInReach->ApplyDockingProgressionFlag();
			
			// Play sound for when the boat stops
			BoatSoundStop();
			
			// Repossess player character
			AController* PlayerController = GetController();
			PlayerController->Possess(PlayerCharacter);
			
			// Fix camera after repossessing player
			FixCameraAfterRepossessingPlayer();
			
			// Player character found, no need to go through the rest of the attached actors
			return;
		}
	}
}

void ABoatFunctionality::SetDockInReach(ADock* Dock)
{
	DockInReach = Dock;
}

void ABoatFunctionality::RemoveDockInReach()
{
	if (DockInReach)
	{
		DockInReach->HideEnterDockPrompt();
	}

	DockInReach = nullptr;
}

// ------------------------------------------------------------------- UI -------------------------------------------------------------------

void ABoatFunctionality::ShowEnterBoatPrompt()
{
	if (EnterBoatPromptWidget || !EnterBoatPromptWidgetClass)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
	{
		return;
	}

	EnterBoatPromptWidget = CreateWidget<UUserWidget>(PlayerController, EnterBoatPromptWidgetClass);

	if (EnterBoatPromptWidget)
	{
		EnterBoatPromptWidget->AddToViewport();
	}
}

void ABoatFunctionality::HideEnterBoatPrompt()
{
	if (EnterBoatPromptWidget)
	{
		EnterBoatPromptWidget->RemoveFromParent();
		EnterBoatPromptWidget = nullptr;
	}
}

void ABoatFunctionality::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// When the player enters the boat, remove the "enter boat" prompt immediately.
	HideEnterBoatPrompt();

	// Also hide dock prompt if the boat is still near a dock.
	if (DockInReach)
	{
		DockInReach->HideEnterDockPrompt();
	}
}

// ---------------------------------------------------------------- PROGRESSION ----------------------------------------------------------------

bool ABoatFunctionality::CanPlayerEnterBoat() const
{
	if (bRequiresFlagToEnterBoat)
	{
		if (RequiredFlagToEnterBoat.IsNone())
		{
			UE_LOG(LogTemp, Warning, TEXT("Boat requires a flag, but RequiredFlagToEnterBoat is None."));
			return false;
		}

		AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
		);

		if (!ProgressionManager)
		{
			UE_LOG(LogTemp, Warning, TEXT("Boat could not find ProgressionManager."));
			return false;
		}

		if (!ProgressionManager->HasFlag(RequiredFlagToEnterBoat))
		{
			UE_LOG(LogTemp, Warning, TEXT("Boat locked. Missing flag: %s"), *RequiredFlagToEnterBoat.ToString());
			return false;
		}
	}

	if (DockInReach && !DockInReach->CanLeaveDock())
	{
		return false;
	}

	return true;
}

void ABoatFunctionality::ShowCannotEnterBoatMessage() const
{
	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (!DialogueManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boat could not find DialogueManager."));
		return;
	}

	if (DockInReach && !DockInReach->CanLeaveDock())
	{
		DialogueManager->ShowMessage(DockInReach->GetCannotLeaveDockMessage());
		return;
	}

	if (CannotEnterBoatMessage.IsEmpty())
	{
		DialogueManager->ShowMessage(FText::FromString(TEXT("I should finish helping here before leaving.")));
	}
	else
	{
		DialogueManager->ShowMessage(CannotEnterBoatMessage);
	}
}