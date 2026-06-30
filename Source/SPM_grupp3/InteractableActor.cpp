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

	/*
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
	*/
}

/*
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
*/

/*
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
*/

void AInteractableActor::Interact()
{
	UE_LOG(LogTemp, Warning, TEXT("Actor interacted!"));
}

void AInteractableActor::SetPromptVisible(bool bVisible)
{
	// This interactable opts out of the prompt (e.g. it relies on its outline instead), or has no
	// prompt widget class set. Either way: never show a prompt, just make sure none is lingering.
	if (bVisible && (!bShowInteractPrompt || !InteractPromptWidgetClass))
	{
		bVisible = false;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!PlayerController)
	{
		return;
	}

	if (bVisible)
	{
		if (!InteractPromptWidgetInstance && InteractPromptWidgetClass)
		{
			InteractPromptWidgetInstance = CreateWidget<UUserWidget>(
				PlayerController,
				InteractPromptWidgetClass
			);
		}

		if (InteractPromptWidgetInstance && !InteractPromptWidgetInstance->IsInViewport())
		{
			InteractPromptWidgetInstance->AddToViewport();
		}
	}
	else
	{
		if (InteractPromptWidgetInstance)
		{
			InteractPromptWidgetInstance->RemoveFromParent();
			InteractPromptWidgetInstance = nullptr;
		}
	}
}