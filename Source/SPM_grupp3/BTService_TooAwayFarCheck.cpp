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

//A service where we check every tick if the AI is too far away from the player where we set the chosen
//key to either true or false. The key and AcceptableDistance is chosen in the behavior tree. 
void UBTService_TooAwayFarCheck::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	PlayerCharacter = UGameplayStatics::GetPlayerCharacter(OwnerComp.GetWorld(), 0);
	Blackboard = OwnerComp.GetBlackboardComponent();
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!PlayerCharacter || !Blackboard || !Controller) return;
	
	APawn* AIPawn = Controller->GetPawn();
	
	PlayerLocation = PlayerCharacter->GetActorLocation();
	AILocation = AIPawn->GetActorLocation();
	
	//Checks the distance between two 
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
