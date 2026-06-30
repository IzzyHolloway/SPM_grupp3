// Fill out your copyright notice in the Description page of Project Settings.

#include "CutsceneFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UCutsceneFunctionLibrary::FadeScreen(const UObject* WorldContextObject, bool bToBlack, float Duration)
{
	APlayerCameraManager* Mgr = UGameplayStatics::GetPlayerCameraManager(WorldContextObject, 0);
	if (!Mgr)
	{
		return;
	}

	if (bToBlack)
	{
		// 0 (clear) -> 1 (black), hold black when finished so the screen stays dark for the cut.
		Mgr->StartCameraFade(0.f, 1.f, Duration, FLinearColor::Black, false, /*bHoldWhenFinished*/ true);
	}
	else
	{
		// 1 (black) -> 0 (clear); don't hold, so the fade clears completely.
		Mgr->StartCameraFade(1.f, 0.f, Duration, FLinearColor::Black, false, /*bHoldWhenFinished*/ false);
	}
}

void UCutsceneFunctionLibrary::FadeCutCameraTo(const UObject* WorldContextObject, AActor* CameraToMove, FVector NewLocation, FRotator NewRotation, float FadeOutTime, float HoldBlackTime, float FadeInTime)
{
	APlayerCameraManager* Mgr = UGameplayStatics::GetPlayerCameraManager(WorldContextObject, 0);
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!Mgr || !World)
	{
		return;
	}

	// 1) Fade to black and hold there.
	Mgr->StartCameraFade(0.f, 1.f, FadeOutTime, FLinearColor::Black, false, /*bHoldWhenFinished*/ true);

	// 2) Once the screen is fully black, snap the camera to the new angle and fade back in. The
	// camera move happens behind black, so the hard cut is invisible. A loose FTimerHandle is fine
	// here -- the timer manager owns the entry and fires it once; weak pointers guard against the
	// camera or manager being torn down before it fires (e.g. level change mid-cutscene).
	const TWeakObjectPtr<AActor> WeakCam = CameraToMove;
	const TWeakObjectPtr<APlayerCameraManager> WeakMgr = Mgr;
	const float InFadeTime = FadeInTime;

	FTimerHandle Handle;
	World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([WeakCam, WeakMgr, NewLocation, NewRotation, InFadeTime]()
	{
		if (WeakCam.IsValid())
		{
			WeakCam->SetActorLocationAndRotation(NewLocation, NewRotation);
		}
		if (WeakMgr.IsValid())
		{
			WeakMgr->StartCameraFade(1.f, 0.f, InFadeTime, FLinearColor::Black, false, /*bHoldWhenFinished*/ false);
		}
	}), FMath::Max(FadeOutTime + HoldBlackTime, 0.01f), /*bLoop*/ false);
}

UCutsceneFadeAction* UCutsceneFadeAction::FadeCameraAndWait(const UObject* WorldContextObject, bool bToBlack, float Duration)
{
	UCutsceneFadeAction* Action = NewObject<UCutsceneFadeAction>();
	Action->WorldPtr = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	Action->bToBlack = bToBlack;
	Action->Duration = Duration;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UCutsceneFadeAction::Activate()
{
	UWorld* World = WorldPtr.Get();
	APlayerCameraManager* Mgr = World ? UGameplayStatics::GetPlayerCameraManager(World, 0) : nullptr;
	if (!World || !Mgr)
	{
		// Nothing to fade -- fire immediately so the graph doesn't stall.
		OnFinished.Broadcast();
		SetReadyToDestroy();
		return;
	}

	if (bToBlack)
	{
		Mgr->StartCameraFade(0.f, 1.f, Duration, FLinearColor::Black, false, /*bHoldWhenFinished*/ true);
	}
	else
	{
		Mgr->StartCameraFade(1.f, 0.f, Duration, FLinearColor::Black, false, /*bHoldWhenFinished*/ false);
	}

	// Continue only once the fade has actually played out.
	World->GetTimerManager().SetTimer(TimerHandle, this, &UCutsceneFadeAction::HandleFadeComplete, FMath::Max(Duration, 0.01f), /*bLoop*/ false);
}

void UCutsceneFadeAction::HandleFadeComplete()
{
	OnFinished.Broadcast();
	SetReadyToDestroy();
}
