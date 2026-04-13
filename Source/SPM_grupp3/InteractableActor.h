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

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
	TObjectPtr<UWidgetComponent> PromptWidget;

public:
	UFUNCTION()
	void Interact();

	void SetPromptVisible(bool bVisible);
};