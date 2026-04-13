#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableActor.generated.h"

UCLASS()
class SPM_GRUPP3_API AInteractableActor : public AActor
{
	GENERATED_BODY()

public:
	AInteractableActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

public:
	UFUNCTION()
	void Interact();
};