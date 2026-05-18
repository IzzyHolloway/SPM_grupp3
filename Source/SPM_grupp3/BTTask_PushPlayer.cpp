// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_PushPlayer.h"

#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_PushPlayer::UBTTask_PushPlayer()
{
	NodeName = "PushPlayer";
}

EBTNodeResult::Type UBTTask_PushPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller)
	{
		return EBTNodeResult::Failed;
	}
	APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();
	ACharacter* PlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	
	if (!AIPawn || !PlayerCharacter)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}
	
	FVector AILocation = AIPawn->GetActorLocation();
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	
	FVector PushDirection = (PlayerLocation - AILocation).GetSafeNormal();
	PlayerCharacter->LaunchCharacter(PushDirection * PushStrength, true, true);
	
	FVector PlayerRight = PlayerCharacter->GetActorRightVector();
	float DotProduct = FVector::DotProduct(PlayerRight, (AILocation - PlayerLocation).GetSafeNormal());
	
	FRotator Rotation = PlayerCharacter->GetActorRotation();
	
	if (DotProduct > 0.0f)
	{
		Rotation.Yaw -= 90.0f;
	}else
	{
		Rotation.Yaw += 90.0f;
	}
	
	PlayerCharacter->SetActorRotation(Rotation);
	
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
	
}
