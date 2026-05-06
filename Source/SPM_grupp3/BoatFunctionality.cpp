// Fill out your copyright notice in the Description page of Project Settings.


#include "BoatFunctionality.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"

#include "CharacterAimi.h"

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
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
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
	UE_LOG(LogTemp, Warning, TEXT("DEBUG: Oh, an overlap began! :D Other actor: %s"), *OtherActor->GetName());
	
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
	// TODO: Communicate to the player character that it's possible to enter the boat now and hand over a reference to myself
}

void ABoatFunctionality::OnEnterTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("DEBUG: Oh, an overlap ended! :D Other actor: %s"), *OtherActor->GetName());
	
	// Check if the overlapping object is the player character
	if (ACharacterAimi* PlayerCharacter = Cast<ACharacterAimi>(OtherActor))
	{
		// Disable entering the boat for the player
		DisableEnteringBoat(PlayerCharacter);
	}
}

void ABoatFunctionality::DisableEnteringBoat(ACharacterAimi* PlayerCharacter)
{
	// TODO: Communicate to the player character that it isn't possible anymore to enter the boat and reset reference to boat to null
}
