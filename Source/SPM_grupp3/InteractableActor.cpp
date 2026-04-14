#include "InteractableActor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"

AInteractableActor::AInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
	PromptWidget->SetupAttachment(RootComponent);
	PromptWidget->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	PromptWidget->SetDrawAtDesiredSize(true);
	PromptWidget->SetWidgetSpace(EWidgetSpace::World);
	PromptWidget->SetVisibility(false);
}

void AInteractableActor::Interact()
{
	UE_LOG(LogTemp, Warning, TEXT("Actor interacted!"));
	//Destroy();
}

void AInteractableActor::SetPromptVisible(bool bVisible)
{
	
	//UE_LOG(LogTemp, Warning, TEXT("SetPromptVisible called on %s"), *GetName());
	if (!IsValid(PromptWidget))
	{
		return;
	}

	PromptWidget->SetVisibility(bVisible);
}