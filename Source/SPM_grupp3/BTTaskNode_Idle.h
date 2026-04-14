// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_Idle.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GRUPP3_API UBTTaskNode_Idle : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTaskNode_Idle();
	
	UPROPERTY(EditAnywhere, Category = "Blackoard")
	FBlackboardKeySelector RandomPosition;
	UPROPERTY(EditAnywhere, Category = "Details")
	float Radius = 500.0f;
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
	UPROPERTY()
	UBlackboardComponent* Blackboard = nullptr;
	
};
