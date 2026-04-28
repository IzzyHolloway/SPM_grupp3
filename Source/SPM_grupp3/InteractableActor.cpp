#include "InteractableActor.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"

AInteractableActor::AInteractableActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
	PromptWidget->SetupAttachment(RootComponent);

	// Use world space so the prompt stays attached above the object in the world.
	PromptWidget->SetWidgetSpace(EWidgetSpace::World);

	// Fixed draw size so every prompt uses the same widget size.
	PromptWidget->SetDrawAtDesiredSize(false);
	PromptWidget->SetDrawSize(PromptDrawSize);

	// Makes the widget visible from both sides, useful if it rotates strangely.
	PromptWidget->SetTwoSided(true);

	// This is important: do not let object scale make the prompt huge/tiny.
	PromptWidget->SetUsingAbsoluteScale(true);
	PromptWidget->SetWorldScale3D(FVector(PromptWorldScale));

	PromptWidget->SetVisibility(false);
}

void AInteractableActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (PromptWidget)
	{
		PromptWidget->SetRelativeLocation(PromptOffset);
		PromptWidget->SetDrawSize(PromptDrawSize);

		// Keep prompt size consistent even if the mesh/actor is scaled.
		PromptWidget->SetUsingAbsoluteScale(true);
		PromptWidget->SetWorldScale3D(FVector(PromptWorldScale));
	}
}

void AInteractableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bPromptFacesCamera || !PromptWidget || !PromptWidget->IsVisible())
	{
		return;
	}

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (!CameraManager)
	{
		return;
	}

	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FVector WidgetLocation = PromptWidget->GetComponentLocation();

	const FVector DirectionToCamera = CameraLocation - WidgetLocation;

	if (!DirectionToCamera.IsNearlyZero())
	{
		PromptWidget->SetWorldRotation(DirectionToCamera.Rotation());
	}
}

void AInteractableActor::Interact()
{
	UE_LOG(LogTemp, Warning, TEXT("Actor interacted!"));
}

void AInteractableActor::SetPromptVisible(bool bVisible)
{
	if (!IsValid(PromptWidget))
	{
		return;
	}

	PromptWidget->SetVisibility(bVisible);
}