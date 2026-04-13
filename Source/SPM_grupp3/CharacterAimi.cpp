// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAimi.h"

// Sets default values
ACharacterAimi::ACharacterAimi()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACharacterAimi::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACharacterAimi::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACharacterAimi::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

