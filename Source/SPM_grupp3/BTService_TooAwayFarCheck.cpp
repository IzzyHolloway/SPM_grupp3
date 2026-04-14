// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_TooAwayFarCheck.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

UBTService_TooAwayFarCheck::UBTService_TooAwayFarCheck()
{
	NodeName = "TooAwayFarCheck";
}

void UBTService_TooAwayFarCheck::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UE_LOG(LogTemp, Warning, TEXT("We are in the script!"));
	PlayerCharacter = UGameplayStatics::GetPlayerCharacter(OwnerComp.GetWorld(), 0);
	Blackboard = OwnerComp.GetBlackboardComponent();
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!PlayerCharacter || !Blackboard || !Controller) return;
	
	APawn* AIPawn = Controller->GetPawn();
	
	PlayerLocation = PlayerCharacter->GetActorLocation();
	AILocation = AIPawn->GetActorLocation();
	float Distance = (PlayerLocation - AILocation).Length();
	UE_LOG(LogTemp, Warning, TEXT("Distance between AI and Player: %f"), Distance);
	
	if (Distance > AcceptableDistance)
	{
		Blackboard->SetValueAsBool(TooFarAwayKey.SelectedKeyName, true);
	}else
	{
		Blackboard->SetValueAsBool(TooFarAwayKey.SelectedKeyName, false);
	}
	
}
