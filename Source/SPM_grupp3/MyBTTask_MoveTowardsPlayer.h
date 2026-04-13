// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "MyBTTask_MoveTowardsPlayer.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GRUPP3_API UMyBTTask_MoveTowardsPlayer : public UBTTaskNode
{
	GENERATED_BODY()
	
	public:
	UMyBTTask_MoveTowardsPlayer();
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MoveLocationKey;
	
	protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
