// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dock.generated.h"

class ABoatFunctionality;
class UBoxComponent;

UCLASS()
class SPM_GRUPP3_API ADock : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADock();
	
	// Reacts to the OnComponentBeginOverlap event of the EnterTrigger (for the player to enter the boat) - calls EnableEnteringBoat()
	UFUNCTION()
	void OnExitBoatTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// Reacts to the OnComponentEndOverlap event of the EnterTrigger (for the player to enter the boat) - calls DisableEnteringBoat()
	UFUNCTION()
	void OnExitBoatTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	// Returns offset the character should have to the dock's coordinate center when it gets placed on the dock
	UFUNCTION()
	FVector GetCharacterPositionOffset() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction with boat")
	TObjectPtr<UBoxComponent> ExitBoatTrigger;
	
	// Offset the character should have to the dock's coordinate center when it gets placed on the dock
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enter & Exit")
	FVector CharacterPositionOffset = FVector(0.0f, 0.0f, 0.0f);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
private:
	void EnableExitingBoat(ABoatFunctionality* Boat);
	void DisableExitingBoat(ABoatFunctionality* Boat);
	
};
