// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PushPlayer.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GRUPP3_API UBTTask_PushPlayer : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_PushPlayer();
	
	UPROPERTY(EditAnywhere, Category = "Push Settings")
	float PushStrength = 1000.0f;
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
