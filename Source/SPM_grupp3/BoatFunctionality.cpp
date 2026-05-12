// Fill out your copyright notice in the Description page of Project Settings.


#include "BoatFunctionality.h"

#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"

#include "ProgressionManager.h"

#include "CharacterAimi.h"
#include "CharacterPaula.h"
#include "Dock.h"

#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/MovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABoatFunctionality::ABoatFunctionality()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    
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
	MovementComponent->UpdatedComponent = RootComponent;
	
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
		const FVector RotationInput = GetActorRightVector();
		AddActorWorldRotation(FRotator(0.f, MovementValue.X * RotationSpeed * GetWorld()->GetDeltaSeconds(), 0.f));
		
		// Apply forward and back movement
		AddMovementInput(GetActorForwardVector(), MovementValue.Y);
	}
}

void ABoatFunctionality::Look(const FInputActionValue& Value)
{
	// 2D Vector of look values returned from the input action
	const FVector2D LookValue = Value.Get<FVector2D>();

	AddControllerYawInput(LookValue.X);
	AddControllerPitchInput(LookValue.Y);
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
	// UE_LOG(LogTemp, Warning, TEXT("DEBUG: Oh, an overlap began! :D Other actor: %s"), *OtherActor->GetName());
	
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
	
	// Player only allowed to use boat after lantern is lit
	if (!ProgressionManager->HasFlag(TEXT("LitLantern")))
	{
		UE_LOG(LogTemp, Warning, TEXT("LitLantern is not active. Player needs to craft a lit lantern"));
		
		PlayerCharacter->RemoveBoatInReach();
		return;
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

	// Set camera position
	SetCameraPositionWhenExiting(Camera);
	
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
			PlayerCharacter->SetActorLocation(DockInReach->GetActorLocation() + DockInReach->GetCharacterPositionOffset());
			
			// Hide dock prompt because we are exiting the boat now
			DockInReach->HideEnterDockPrompt();
			
			// Add the dock's arrival/progression flag, for example ArrivedIsland1, ArrivedIsland2, etc.
			DockInReach->ApplyDockingProgressionFlag();
			
			// Repossess player character
			AController* PlayerController = GetController();
			PlayerController->Possess(PlayerCharacter);
			
			// Fix camera after repossessing player
			FixCameraAfterRepossessingPlayer();
			
			// BoatSound-stop
			BoatSoundStop();
			
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