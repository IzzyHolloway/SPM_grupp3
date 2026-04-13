#include "InteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

AInteractableActor::AInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
}

void AInteractableActor::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractableActor::Interact()
{
	UE_LOG(LogTemp, Warning, TEXT("Interactable actor was interacted with"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Actor interacted!"));
	}

	Destroy();
}