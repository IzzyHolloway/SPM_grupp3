// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTaskNode_LookForPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UBTTaskNode_LookForPlayer::UBTTaskNode_LookForPlayer()
{
	NodeName = "LookForPlayer";
}

EBTNodeResult::Type UBTTaskNode_LookForPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	PlayerCharacter = UGameplayStatics::GetPlayerCharacter(OwnerComp.GetWorld(), 0);
	Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard || !PlayerCharacter) return EBTNodeResult::Failed;
	
	PlayerLocation = PlayerCharacter->GetActorLocation();
	
	Blackboard->SetValueAsVector(PlayerLocationKey.SelectedKeyName, PlayerLocation);
	return EBTNodeResult::Succeeded;
}
