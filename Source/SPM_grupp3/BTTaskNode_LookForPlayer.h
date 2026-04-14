// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_LookForPlayer.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GRUPP3_API UBTTaskNode_LookForPlayer : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTaskNode_LookForPlayer();
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PlayerLocationKey;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
	UPROPERTY()
	ACharacter* PlayerCharacter = nullptr;
	UPROPERTY()
	UBlackboardComponent* Blackboard = nullptr;
	UPROPERTY()
	FVector PlayerLocation = FVector::ZeroVector;
};
