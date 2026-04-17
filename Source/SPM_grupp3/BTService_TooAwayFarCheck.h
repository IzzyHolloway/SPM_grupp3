// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_TooAwayFarCheck.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GRUPP3_API UBTService_TooAwayFarCheck : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_TooAwayFarCheck();
	
	UPROPERTY(EditAnywhere, Category = "Blackboard Key")
	FBlackboardKeySelector TooFarAwayKey;
	
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
private:
	UPROPERTY()
	ACharacter* PlayerCharacter = nullptr;
	UPROPERTY()
	UBlackboardComponent* Blackboard = nullptr;
	FVector PlayerLocation = FVector(0.0f, 0.0f, 0.0f);
	FVector AILocation = FVector(0.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnyWhere, Category = "Details")
	float AcceptableDistance = 500.0f;
	
};
