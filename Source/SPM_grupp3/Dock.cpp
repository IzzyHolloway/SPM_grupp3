// Fill out your copyright notice in the Description page of Project Settings.


#include "Dock.h"
#include "BoatFunctionality.h"
#include "DelayAction.h"
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
	
	ExitBoatTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitBoatTrigger"));
	ExitBoatTrigger->SetupAttachment(MeshComponent);
	
	// Subscribe OnEnterTriggerBeginOverlap function to the OnComponentBeginOverlap event of the enter trigger box
	ExitBoatTrigger->OnComponentBeginOverlap.AddDynamic(this, &ADock::OnExitBoatTriggerBeginOverlap);
	
	// Subscribe OnEnterTriggerEndOverlap function to the OnComponentEndOverlap event of the enter trigger box
	ExitBoatTrigger->OnComponentEndOverlap.AddDynamic(this, &ADock::OnExitBoatTriggerEndOverlap);

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
		ExitBoatTrigger->GetOverlappingActors(OverlappingBoats, ABoatFunctionality::StaticClass());
		
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

void ADock::OnExitBoatTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the overlapping object is the boat
	if (ABoatFunctionality* Boat = Cast<ABoatFunctionality>(OtherActor))
	{
		GEngine->AddOnScreenDebugMessage(
			-1,                // Key (-1 means add a new message)
			5.0f,              // Display time in seconds
			FColor::White,     // Text color
			TEXT("Press E to exit the boat!") // Message
		);
		
		// Enable exiting the boat
		EnableExitingBoat(Boat);
	}
}

void ADock::OnExitBoatTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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

void ADock::EnableExitingBoat(ABoatFunctionality* Boat)
{
	// Progression Flag added after solving Intro Puzzle and jumping off at Island1
	// TODO: Progression stuff
	
	// Hand over a reference to myself to the boat to enable exiting it
	Boat->SetDockInReach(this);
}

void ADock::DisableExitingBoat(ABoatFunctionality* Boat)
{
	// Remove the reference to myself in the boat to disable exiting it
	Boat->RemoveDockInReach();
}
