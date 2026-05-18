// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckForPlayer.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GRUPP3_API UBTService_CheckForPlayer : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_CheckForPlayer();
	
	UPROPERTY(EditAnywhere, Category = "Blackboard Key")
	FBlackboardKeySelector IsPlayerInWater;
	
	virtual void SetIsPlayerOnBoat(bool InIsPlayerOnBoat);

protected:
	virtual void IsPlayerOnBoat(UBehaviorTreeComponent& OwnerComp,bool InWater);
	
private:
	UPROPERTY()
	UBlackboardComponent* Blackboard = nullptr;
};
