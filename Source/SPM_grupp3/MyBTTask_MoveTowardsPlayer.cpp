// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBTTask_MoveTowardsPlayer.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

UMyBTTask_MoveTowardsPlayer::UMyBTTask_MoveTowardsPlayer()
{
	
}

EBTNodeResult::Type UMyBTTask_MoveTowardsPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (Player == nullptr) return EBTNodeResult::Failed;
	
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard == nullptr) return EBTNodeResult::Failed;
	
	Blackboard->SetValueAsVector(MoveLocationKey.SelectedKeyName, Player->GetActorLocation());
	
	return EBTNodeResult::Succeeded;
}
