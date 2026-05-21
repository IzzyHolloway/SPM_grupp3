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
	
}

// Called every frame
void ABoatFunctionality::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// ---------------------------------- INPUT ----------------------------------

// Called to bind functionality to input
void ABoatFunctionality::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind Movement and Rotation Actions
		EnhancedInputComponent->BindAction(MoveRotateAction, ETriggerEvent::Triggered, this, &ABoatFunctionality::MoveRotate);
		
		// Bind Look Actions
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABoatFunctionality::Look);
		
		// Bind Interact Actions
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ABoatFunctionality::Interact);
	}
}

// -------------------------------- MOVEMENT --------------------------------

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
		
		// Make sure the boat stays on the same height
		// TODO: Quick fix, replace later
		// SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, 60.0));
	}
}

void ABoatFunctionality::Look(const FInputActionValue& Value)
{
	// 2D Vector of look values returned from the input action
	const FVector2D LookValue = Value.Get<FVector2D>();

	// Check if the controller possessing this Actor is valid
	if (Controller)
	{
		AddControllerYawInput(LookValue.X);
		AddControllerPitchInput(LookValue.Y);
	}
}

// --------------------------------- INTERACTION ---------------------------------

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
	/*
	* Only for debugging:
	else
	{
		GEngine->AddOnScreenDebugMessage(
			-1,                // Key (-1 means add a new message)
			5.0f,              // Display time in seconds
			FColor::White,     // Text color
			TEXT("There is no dock to tie up at!") // Message
		);
	}
	*/
}

// -------------------------------- ENTER & EXIT --------------------------------

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
	
	// Find ProgressionManager in level
	// TO be able to board the boat, it is enough to see if the player has lit the lantern
	
	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass()));

	if (!ProgressionManager)
	{
		// Boat coult not find ProgressionManager
		
		UE_LOG(LogTemp, Warning, TEXT("Boat couldn't find ProgressionManager"));
		
		PlayerCharacter->RemoveBoatInReach();
		return;
	}
	
	// Player is only allowed to enter the boat if the required progression flag is active.
	if (bRequiresFlagToEnterBoat)
	{
		if (RequiredFlagToEnterBoat.IsNone())
		{
			UE_LOG(LogTemp, Warning, TEXT("Boat requires a flag, but RequiredFlagToEnterBoat is None."));
			PlayerCharacter->RemoveBoatInReach();
			HideEnterBoatPrompt();
			return;
		}

		if (!ProgressionManager->HasFlag(RequiredFlagToEnterBoat))
		{
			UE_LOG(LogTemp, Warning, TEXT("Boat locked. Missing flag: %s"), *RequiredFlagToEnterBoat.ToString());

			PlayerCharacter->RemoveBoatInReach();
			HideEnterBoatPrompt();

			/*
			// Temporary feedback. Later this can call DialogueManager or your message system.
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					3.0f,
					FColor::White,
					CannotEnterBoatMessage.IsEmpty()
	? TEXT("I should finish helping here before leaving.")
	: CannotEnterBoatMessage.ToString()
				);
			}
			*/
			
			// Dialogue that says they can't board the boat yet
			ADialogueManager* DialogueManager = Cast<ADialogueManager>(
	UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
);

			if (DialogueManager)
			{
				DialogueManager->ShowMessage(CannotEnterBoatMessage);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Boat could not find DialogueManager."));
			}

			return;
		}
	}
	
	// If the LitLantern flag is true, the player is allowed to enter the boat
	//PlayerCharacter->SetBoatInReach(this);
	
	/*
	* Only for debugging:
	GEngine->AddOnScreenDebugMessage(
			-1,                // Key (-1 means add a new message)
			5.0f,              // Display time in seconds
			FColor::White,     // Text color
			TEXT("Press E to enter the boat!") // Message
		);
	 */
	
	// Check if the current dock allows the player to leave this island.
	if (DockInReach && !DockInReach->CanLeaveDock())
	{
		UE_LOG(LogTemp, Warning, TEXT("Boat locked by dock leave requirement."));

		PlayerCharacter->RemoveBoatInReach();
		HideEnterBoatPrompt();

		ADialogueManager* DialogueManager = Cast<ADialogueManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
		);

		if (DialogueManager)
		{
			DialogueManager->ShowMessage(DockInReach->GetCannotLeaveDockMessage());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Boat could not find DialogueManager."));
		}

		return;
	}
	
	/*
	// If the boat is currently at a dock, ask the dock if the player is allowed to leave.
	if (DockInReach && !DockInReach->CanLeaveDock())
	{
		PlayerCharacter->RemoveBoatInReach();
		HideEnterBoatPrompt();

		UE_LOG(LogTemp, Warning, TEXT("Boat locked by dock leave requirement."));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.0f,
				FColor::White,
				DockInReach->GetCannotLeaveDockMessage().ToString()
			);
		}

		return;
	}
	*/
	
	// Hand over a reference to myself to the player character to enable it to enter the boat
	PlayerCharacter->SetBoatInReach(this);
	
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
	/*
	* Only for debugging:
	GEngine->AddOnScreenDebugMessage(
			-1,                // Key (-1 means add a new message)
			5.0f,              // Display time in seconds
			FColor::White,     // Text color
			TEXT("Boat is out of reach.") // Message
		);
	 */
	
	// Remove the reference to myself in the player character to disable entering the boat
	PlayerCharacter->RemoveBoatInReach();
	
	HideEnterBoatPrompt();
}

// Returns offset the character should have to the boat's coordinate center when it gets placed in the boat
FVector ABoatFunctionality::GetCharacterPositionOffset() const
{
	return CharacterPositionOffset;
}

void ABoatFunctionality::ExitBoat()
{
	HideEnterBoatPrompt();

	if (DockInReach)
	{
		DockInReach->HideEnterDockPrompt();
	}
	
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
	
	//---------------------- MADDE AI FOR WATER ----------------------
	//This gets an array with all the actors that has the tag WaterAI on it.
	TArray<AActor*> AICharacters;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("WaterAI"), AICharacters);
	
	UE_LOG(LogTemp, Warning, TEXT("We have exited the boat!"));
	
	//If we found an actor we will get its controller. 
	if (AICharacters.Num() > 0)
	{
		AAIController* AIController = Cast<AAIController>(Cast<APawn>(AICharacters[0])->GetController());
		if (AIController)
		{
			AIController->GetBlackboardComponent()->SetValueAsBool("IsInBoat", false);
		}
	}
	
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
			// Detach player character
			PlayerCharacter->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			
			// Move player character on top of the pier
			PlayerCharacter->SetActorLocation(CurrentDockInReach->GetActorLocation() + CurrentDockInReach->GetCharacterPositionOffset());
			
			// Hide dock prompt because we are exiting the boat now
			CurrentDockInReach->HideEnterDockPrompt();
			
			// Add the dock's arrival/progression flag, for example ArrivedIsland1, ArrivedIsland2, etc.
			CurrentDockInReach->ApplyDockingProgressionFlag();
			
			//Zoey moved this part up
			// BoatSound-stop
			BoatSoundStop();
			//Zoey end
			
			
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