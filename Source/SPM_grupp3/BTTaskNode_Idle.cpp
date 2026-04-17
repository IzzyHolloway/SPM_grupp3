// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTaskNode_Idle.h"

#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "BTService_TooAwayFarCheck.h"
#include "AIController.h"

UBTTaskNode_Idle::UBTTaskNode_Idle()
{
	NodeName = "Idle";
}

//A task that will get a random point around the player with the same radius as the AcceptableDistance 
//and where the chose key will be set. The key and Radius is set in the behavior tree. 
EBTNodeResult::Type UBTTaskNode_Idle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UWorld* World = OwnerComp.GetWorld();
	if (!World) return EBTNodeResult::Failed;
	
	ACharacter* Origin = UGameplayStatics::GetPlayerCharacter(World, 0);
	Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Origin || !Blackboard) return EBTNodeResult::Failed;
	
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
	if (!NavSystem) return EBTNodeResult::Failed;

	FNavLocation RandomPoint;
	
	bool bFound = NavSystem->GetRandomReachablePointInRadius(Origin->GetActorLocation(), Radius, RandomPoint);
	if (!bFound) return EBTNodeResult::Failed;
	
	Blackboard->SetValueAsVector(RandomPosition.SelectedKeyName, RandomPoint.Location);
	
	return EBTNodeResult::Succeeded;
}
