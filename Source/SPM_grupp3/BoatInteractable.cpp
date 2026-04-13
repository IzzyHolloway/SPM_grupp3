// Fill out your copyright notice in the Description page of Project Settings.

#include "BoatInteractable.h"
#include "CharacterAimi.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void ABoatInteractable::Interact()
{
	ACharacterAimi* PlayerCharacter = Cast<ACharacterAimi>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!PlayerCharacter)
	{
		return;
	}

	if (PlayerCharacter->HasRequiredItems())
	{
		UE_LOG(LogTemp, Warning, TEXT("Player can board the boat"));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("All items found. You can board the boat."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Player is missing items"));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Hmm... It feels like I'm missing something..."));
		}
	}
}