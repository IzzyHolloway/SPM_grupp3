// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_CheckForPlayer.h"

UBTService_CheckForPlayer::UBTService_CheckForPlayer()
{
	NodeName = "CheckForPlayer";
}

void UBTService_CheckForPlayer::SetIsPlayerOnBoat(bool InIsPlayerOnBoat)
{
	IsPlayer
}

UBTService_CheckForPlayer::IsPlayerOnBoat(UBehaviorTreeComponent& OwnerComp, bool InWater)
{
	Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard) return;
	
	Blackboard->SetValueAsBool(IsPlayerInWater.SelectedKeyName, InWater);
}