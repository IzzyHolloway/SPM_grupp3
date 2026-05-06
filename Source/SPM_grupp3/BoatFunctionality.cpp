// Fill out your copyright notice in the Description page of Project Settings.


#include "BoatFunctionality.h"

#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"

#include "CharacterAimi.h"
#include "CharacterPaula.h"

// Sets default values
ABoatFunctionality::ABoatFunctionality()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	MeshComponent->SetupAttachment(RootComponent);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 310.f;
	SpringArm->bUsePawnControlRotation = true;
	
	Camera = CreateDefaultSubobject<UCineCameraComponent>(TEXT("FollowCamera"));
	Camera->SetFilmbackPresetByName(TEXT("16:9 DSLR"));
	Camera->SetCurrentFocalLength(18);
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	
	
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
	
	// Display a debug message for five seconds. 
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("DEBUG: We are using BoatFunctionality."));
	
}

// Called every frame
void ABoatFunctionality::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABoatFunctionality::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind Movement and Rotation Actions
		EnhancedInputComponent->BindAction(MoveRotateAction, ETriggerEvent::Triggered, this, &ABoatFunctionality::MoveRotate);
		
		// Bind Look Actions
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABoatFunctionality::Look);
	}
}

// ---------------------------------- INPUT ----------------------------------

void ABoatFunctionality::MoveRotate(const FInputActionValue& Value)
{
	// 2D Vector of movement values returned from the input action
	const FVector2D MovementValue = Value.Get<FVector2D>();
	
	// Check if the controller possessing this Actor is valid
	if (Controller)
	{
		// Apply rotation input
		const FVector RotationInput = GetActorRightVector();
		
		// Apply forward and back movement
		const FVector MovementInput = GetActorForwardVector();
		AddMovementInput(MovementInput, MovementValue.Y);
	}
}

void ABoatFunctionality::Look(const FInputActionValue& Value)
{
	// 2D Vector of look values returned from the input action
	const FVector2D LookValue = Value.Get<FVector2D>();

	AddControllerYawInput(LookValue.X);
	AddControllerPitchInput(LookValue.Y);
}

// -------------------------------- ENTER & EXIT --------------------------------

// Reacts to the OnComponentBeginOverlap event of the EnterTrigger (for the player to enter the boat) - calls EnableEnteringBoat()
void ABoatFunctionality::OnEnterTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// UE_LOG(LogTemp, Warning, TEXT("DEBUG: Oh, an overlap began! :D Other actor: %s"), *OtherActor->GetName());
	
	// Check if the overlapping object is the player character
	if (ACharacterPaula* PlayerCharacter = Cast<ACharacterPaula>(OtherActor))
	{
		// Enable entering the boat for the player
		EnableEnteringBoat(PlayerCharacter);
	}
}

// Communicates to the player character that entering the boat is possible now and hands over a reference to this boat
void ABoatFunctionality::EnableEnteringBoat(ACharacterPaula* PlayerCharacter)
{
	GEngine->AddOnScreenDebugMessage(
			-1,                // Key (-1 means add a new message)
			5.0f,              // Display time in seconds
			FColor::White,     // Text color
			TEXT("Press E to enter the boat!") // Message
		);
	
	// Hand over a reference to myself to the player character to enable it to enter the boat
	PlayerCharacter->SetBoatInReach(this);
}

void ABoatFunctionality::OnEnterTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if the overlapping object is the player character
	if (ACharacterPaula* PlayerCharacter = Cast<ACharacterPaula>(OtherActor))
	{
		// Disable entering the boat for the player
		DisableEnteringBoat(PlayerCharacter);
	}
}

// Communicates to the player character that it isn't possible anymore to enter the boat and removes the reference to this boat
void ABoatFunctionality::DisableEnteringBoat(ACharacterPaula* PlayerCharacter)
{	
	GEngine->AddOnScreenDebugMessage(
			-1,                // Key (-1 means add a new message)
			5.0f,              // Display time in seconds
			FColor::White,     // Text color
			TEXT("Boat is out of reach.") // Message
		);
	
	PlayerCharacter->RemoveBoatInReach();
}

// Returns offset the character should have to the boat's coordinate center when it gets placed in the boat
FVector ABoatFunctionality::GetCharacterPositionOffset() const
{
	return CharacterPositionOffset;
}
