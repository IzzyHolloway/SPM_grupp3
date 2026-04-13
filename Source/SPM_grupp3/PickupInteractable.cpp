// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupInteractable.h"
#include "CharacterAimi.h"
#include "Kismet/GameplayStatics.h"

void APickupInteractable::Interact()
{
	ACharacterAimi* PlayerCharacter = Cast<ACharacterAimi>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!PlayerCharacter)
	{
		return;
	}

	PlayerCharacter->AddCollectedItem(1);

	UE_LOG(LogTemp, Warning, TEXT("Pickup collected"));

	Destroy();
}