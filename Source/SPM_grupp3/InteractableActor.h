#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableActor.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;

UCLASS()
class SPM_GRUPP3_API AInteractableActor : public AActor
{
	GENERATED_BODY()

public:
	AInteractableActor();

	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
	TObjectPtr<UWidgetComponent> PromptWidget;

	//Local position of the prompt above the object. Adjust per Blueprint.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Prompt")
	FVector PromptOffset = FVector(50.f, 0.f, 120.f);

	//Pixel draw size of the widget content.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Prompt")
	FVector2D PromptDrawSize = FVector2D(550.f, 550.f);

	// World scale of the prompt. Lower = smaller in the world.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Prompt")
	float PromptWorldScale = 0.1f;

	//If true, the prompt rotates to face the camera.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Prompt")
	bool bPromptFacesCamera = true;

public:
	UFUNCTION()
	virtual void Interact();

	void SetPromptVisible(bool bVisible);
};