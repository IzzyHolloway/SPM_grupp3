// Fill out your copyright notice in the Description page of Project Settings.


#include "Dock.h"
#include "BoatFunctionality.h"
#include "DelayAction.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

// Sets default values
ADock::ADock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	MeshComponent->SetupAttachment(RootComponent);
	
	// -------------------------------- INTERACTION WITH BOAT --------------------------------
	
	ExitBoatTriggerRight = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitBoatTriggerRight"));
	ExitBoatTriggerRight->SetupAttachment(MeshComponent);
	
	ExitBoatTriggerLeft = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitBoatTriggerLeft"));
	ExitBoatTriggerLeft->SetupAttachment(MeshComponent);
	
	// Subscribe OnEnterTriggerBeginOverlap function to the OnComponentBeginOverlap event of the enter trigger box
	ExitBoatTriggerRight->OnComponentBeginOverlap.AddDynamic(this, &ADock::OnExitBoatTriggerRightBeginOverlap);
	
	// Subscribe OnEnterTriggerEndOverlap function to the OnComponentEndOverlap event of the enter trigger box
	ExitBoatTriggerRight->OnComponentEndOverlap.AddDynamic(this, &ADock::OnExitBoatTriggerRightEndOverlap);
	
	// Subscribe OnEnterTriggerBeginOverlap function to the OnComponentBeginOverlap event of the enter trigger box
	ExitBoatTriggerLeft->OnComponentBeginOverlap.AddDynamic(this, &ADock::OnExitBoatTriggerLeftBeginOverlap);
	
	// Subscribe OnEnterTriggerEndOverlap function to the OnComponentEndOverlap event of the enter trigger box
	ExitBoatTriggerLeft->OnComponentEndOverlap.AddDynamic(this, &ADock::OnExitBoatTriggerLeftEndOverlap);

}

// Called when the game starts or when spawned
void ADock::BeginPlay()
{
	Super::BeginPlay();
	
	// Check if the boat is already in the trigger zone for exiting the boat when the game starts
	
	// Wait for next tick: At BeginPlay, the engine hasn't checked for overlapping actors yet
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		TArray<AActor*> OverlappingBoats;
		ExitBoatTriggerRight->GetOverlappingActors(OverlappingBoats, ABoatFunctionality::StaticClass());
		
		for (AActor* Actor : OverlappingBoats)
		{
			if (ABoatFunctionality* Boat = Cast<ABoatFunctionality>(Actor))
			{
				EnableExitingBoat(Boat);
			}
		}
	});
}

// Called every frame
void ADock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Careful: Tick is turned off!
}

void ADock::OnExitBoatTriggerRightBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the overlapping object is the boat
	if (ABoatFunctionality* Boat = Cast<ABoatFunctionality>(OtherActor))
	{
		/*
		 * Only for debugging:
		GEngine->AddOnScreenDebugMessage(
			-1,                // Key (-1 means add a new message)
			5.0f,              // Display time in seconds
			FColor::White,     // Text color
			TEXT("Press E to exit the boat!") // Message
		);
		*/
		
		// Safe the docking spot corresponding to the trigger box
		GEngine->AddOnScreenDebugMessage(
			-1,                // Key (-1 means add a new message)
			5.0f,              // Display time in seconds
			FColor::White,     // Text color
			TEXT("Current Docking Spot is the right one now") // Message
		);
		CurrentDockingSpotPosition = &RightDockingSpotPosition;
		CurrentDockingSpotRotation = &RightDockingSpotRotation;
		
		
		// Enable exiting the boat
		EnableExitingBoat(Boat);
	}
}

void ADock::OnExitBoatTriggerLeftBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the overlapping object is the boat
	if (ABoatFunctionality* Boat = Cast<ABoatFunctionality>(OtherActor))
	{
		/*
		 * Only for debugging:
		GEngine->AddOnScreenDebugMessage(
			-1,                // Key (-1 means add a new message)
			5.0f,              // Display time in seconds
			FColor::White,     // Text color
			TEXT("Press E to exit the boat!") // Message
		);
		*/
		
		// Safe the docking spot corresponding to the trigger box
		GEngine->AddOnScreenDebugMessage(
			-1,                // Key (-1 means add a new message)
			5.0f,              // Display time in seconds
			FColor::White,     // Text color
			TEXT("Current Docking Spot is the left one now") // Message
		);
		CurrentDockingSpotPosition = &LeftDockingSpotPosition;
		CurrentDockingSpotRotation = &RightDockingSpotRotation;
		
		// Enable exiting the boat
		EnableExitingBoat(Boat);
	}
}

void ADock::OnExitBoatTriggerRightEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if the overlapping object is the boat
	if (ABoatFunctionality* Boat = Cast<ABoatFunctionality>(OtherActor))
	{		
		DisableExitingBoat(Boat);
	}
}

void ADock::OnExitBoatTriggerLeftEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if the overlapping object is the boat
	if (ABoatFunctionality* Boat = Cast<ABoatFunctionality>(OtherActor))
	{		
		DisableExitingBoat(Boat);
	}
}

FVector ADock::GetCharacterPositionOffset() const
{
	return CharacterPositionOffset;
}

FVector ADock::GetDockingSpotPosition() const
{
	return *CurrentDockingSpotPosition;
}

FVector ADock::GetDockingSpotRotation() const
{
	return *CurrentDockingSpotRotation;
}

void ADock::EnableExitingBoat(ABoatFunctionality* Boat)
{
	// Progression Flag added after solving Intro Puzzle and jumping off at Island1
	// TODO: Progression stuff
	
	if (!Boat)
	{
		return;
	}

	// If this dock has a required progression flag,
	// check that the player has unlocked it before allowing docking.
	if (!RequiredFlagToDock.IsNone())
	{
		AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
		);

		if (!ProgressionManager)
		{
			UE_LOG(LogTemp, Warning, TEXT("Dock could not find ProgressionManager."));
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					5.0f,
					FColor::Red,
					TEXT("Dock error: ProgressionManager not found.")
				);
			}

			// Do not allow docking if progression cannot be checked.
			Boat->RemoveDockInReach();
			return;
		}

		if (!ProgressionManager->HasFlag(RequiredFlagToDock))
		{
			
			UE_LOG(LogTemp, Warning, TEXT("The flag required is FALSE"));

			// Do not give the boat a dock reference.
			// This prevents the player from exiting here.
			Boat->RemoveDockInReach();
			return;
		}
	}

	// The dock is unlocked, so the boat is allowed to exit here.
	if (GEngine)
	{
		/*
		 * Only for debugging:
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::White,
			TEXT("Press E to exit the boat!")
		);
		*/
	}

	// Hand over a reference to myself to the boat to enable exiting it.
	Boat->SetDockInReach(this);

	// Only show the dock prompt while the player is actually controlling the boat.
	if (Boat->IsPlayerControlled())
	{
		ShowEnterDockPrompt();
	}
	else
	{
		HideEnterDockPrompt();
	}
}

void ADock::DisableExitingBoat(ABoatFunctionality* Boat)
{
	// Remove the reference to myself in the boat to disable exiting it
	if (Boat)
	{
		Boat->RemoveDockInReach();
	}
	
	HideEnterDockPrompt();
}


// Progression
void ADock::ApplyDockingProgressionFlag()
{
	// If this dock has no arrival flag, do nothing.
	if (FlagToAddWhenDocking.IsNone())
	{
		return;
	}

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dock could not add docking flag: ProgressionManager not found."));
		return;
	}

	// Do not add the same flag again.
	if (ProgressionManager->HasFlag(FlagToAddWhenDocking))
	{
		return;
	}

	ProgressionManager->AddFlag(FlagToAddWhenDocking);

	UE_LOG(LogTemp, Warning, TEXT("Dock added progression flag: %s"), *FlagToAddWhenDocking.ToString());

	if (GEngine)
	{
		/*
		 * Only for debugging:
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Green,
			FString::Printf(TEXT("Added flag: %s"), *FlagToAddWhenDocking.ToString())
		);
		*/
	}
}


void ADock::ShowEnterDockPrompt()
{
	if (EnterDockPromptWidget || !EnterDockPromptWidgetClass)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
	{
		return;
	}

	EnterDockPromptWidget = CreateWidget<UUserWidget>(PlayerController, EnterDockPromptWidgetClass);

	if (EnterDockPromptWidget)
	{
		EnterDockPromptWidget->AddToViewport();
	}
}

void ADock::HideEnterDockPrompt()
{
	if (EnterDockPromptWidget)
	{
		EnterDockPromptWidget->RemoveFromParent();
		EnterDockPromptWidget = nullptr;
	}
}

bool ADock::CanLeaveDock() const
{
	if (!bRequiresFlagToLeaveDock)
	{
		return true;
	}

	if (RequiredFlagToLeaveDock.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("Dock requires a leave flag, but RequiredFlagToLeaveDock is None."));
		return false;
	}

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dock could not find ProgressionManager when checking leave flag."));
		return false;
	}

	return ProgressionManager->HasFlag(RequiredFlagToLeaveDock);
}

FText ADock::GetCannotLeaveDockMessage() const
{
	if (CannotLeaveDockMessage.IsEmpty())
	{
		return FText::FromString(TEXT("I should finish helping here before leaving."));
	}

	return CannotLeaveDockMessage;
}