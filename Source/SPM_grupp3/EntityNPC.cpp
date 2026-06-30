// Fill out your copyright notice in the Description page of Project Settings.
#include "EntityNPC.h"
#include "DialogueManager.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"

AEntityNPC::AEntityNPC()
{
	// Ticking is only used while fading; it is gated by bFading below.
	PrimaryActorTick.bCanEverTick = true;
}

void AEntityNPC::BeginPlay()
{
	Super::BeginPlay();

	// If she has already been talked to (e.g. after a save/reload), she should not be here.
	if (IsTriggerFlagSet())
	{
		bHasFadedOut = true;
		FinishFadeOut();
		return;
	}

	// Listen for the end of any dialogue. Her conversation is driven by the Level Blueprint,
	// so we react to the progression flag it sets rather than to our own Interact().
	BoundManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (BoundManager)
	{
		BoundManager->OnDialogueEnded.AddUniqueDynamic(this, &AEntityNPC::HandleAnyDialogueEnded);
	}
}

void AEntityNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundManager)
	{
		BoundManager->OnDialogueEnded.RemoveDynamic(this, &AEntityNPC::HandleAnyDialogueEnded);
		BoundManager = nullptr;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FadeOutDelayTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AEntityNPC::HandleAnyDialogueEnded()
{
	if (bHasFadedOut)
	{
		return;
	}

	const bool bFlagSet = IsTriggerFlagSet();
	UE_LOG(LogTemp, Warning, TEXT("AEntityNPC: dialogue ended, flag '%s' set = %s"),
		*FadeOutWhenFlagSet.ToString(), bFlagSet ? TEXT("TRUE") : TEXT("FALSE"));

	// Only fade out once her conversation has actually set its flag.
	if (!bFlagSet)
	{
		return;
	}

	bHasFadedOut = true;

	// Wait a moment after the dialogue closes before she starts fading away.
	if (FadeOutDelay > 0.f && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			FadeOutDelayTimerHandle, this, &AEntityNPC::StartFadeOut, FadeOutDelay, false);
	}
	else
	{
		StartFadeOut();
	}
}

AProgressionManager* AEntityNPC::GetProgressionManager() const
{
	return Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);
}

bool AEntityNPC::IsTriggerFlagSet() const
{
	if (FadeOutWhenFlagSet.IsNone())
	{
		return false;
	}

	const AProgressionManager* ProgressionManager = GetProgressionManager();
	return ProgressionManager && ProgressionManager->HasFlag(FadeOutWhenFlagSet);
}

void AEntityNPC::StartFadeOut()
{
	// Stop the particle systems now so they dissipate while she shrinks away.
	DeactivateParticles(this);
	DeactivateParticles(ParticleActor);

	// No fade time requested: remove everything immediately.
	if (FadeOutDuration <= 0.f)
	{
		FinishFadeOut();
		return;
	}

	InitialScale = GetActorScale3D();
	FadeElapsed = 0.f;
	bFading = true;
}

void AEntityNPC::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bFading)
	{
		return;
	}

	FadeElapsed += DeltaSeconds;
	const float Alpha = FMath::Clamp(FadeElapsed / FadeOutDuration, 0.f, 1.f);

	// Shrink the mesh down to nothing as a stand-in for an opacity fade.
	SetActorScale3D(FMath::Lerp(InitialScale, FVector(KINDA_SMALL_NUMBER), Alpha));

	if (Alpha >= 1.f)
	{
		bFading = false;
		FinishFadeOut();
	}
}

void AEntityNPC::FinishFadeOut()
{
	if (bDestroyOnFinish)
	{
		if (ParticleActor)
		{
			ParticleActor->Destroy();
		}
		Destroy();
	}
	else
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);

		if (ParticleActor)
		{
			ParticleActor->SetActorHiddenInGame(true);
		}
	}
}

void AEntityNPC::DeactivateParticles(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	// Deactivate() is virtual on UActorComponent; on Niagara/Cascade FX components it stops
	// spawning new particles so the existing ones finish their lifetime. Harmless on others.
	TInlineComponentArray<UActorComponent*> Components(Actor);
	for (UActorComponent* Component : Components)
	{
		if (Component)
		{
			Component->Deactivate();
		}
	}
}
