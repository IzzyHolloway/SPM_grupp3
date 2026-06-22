// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "CutsceneFunctionLibrary.generated.h"

/**
 * Blueprint-callable helpers for cutscene camera fades. Lets BP_CameraDirector hide the hard
 * cut between camera angles behind a quick fade to/from black, without wiring up the raw
 * Start Camera Fade + Delay nodes by hand. Pure C++; just call the node in the cutscene graph.
 */
UCLASS()
class SPM_GRUPP3_API UCutsceneFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Fades the player's camera to solid black (bToBlack = true, holds black when done) or back
	// from black (bToBlack = false) over Duration seconds. Use this if you want to drive the fade
	// in two halves around your own camera-switch logic.
	UFUNCTION(BlueprintCallable, Category = "Cutscene|Fade", meta = (WorldContext = "WorldContextObject"))
	static void FadeScreen(const UObject* WorldContextObject, bool bToBlack, float Duration = 0.4f);

	// All-in-one camera cut: fades to black, snaps CameraToMove to NewLocation/NewRotation once the
	// screen is fully black, then fades back in. Drop this in place of a hard Set Actor Location +
	// Set Actor Rotation so the angle change isn't choppy. Timing is handled here -- no Delay node
	// needed in Blueprint. Wire in the same camera actor and target transform you use today.
	UFUNCTION(BlueprintCallable, Category = "Cutscene|Fade", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "FadeOutTime,HoldBlackTime,FadeInTime"))
	static void FadeCutCameraTo(const UObject* WorldContextObject, AActor* CameraToMove, FVector NewLocation, FRotator NewRotation, float FadeOutTime = 0.4f, float HoldBlackTime = 0.05f, float FadeInTime = 0.4f);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCutsceneFadeFinished);

/**
 * Latent fade node: tones the player camera to/from black and only fires its "On Finished" exec
 * pin once the fade has fully played out. Unlike FadeScreen (which returns instantly), this one
 * WAITS like a Delay -- so you never have to match a Duration against a separate Delay node.
 * Wire: ... -> Fade Camera And Wait(to black) [On Finished] -> swap angle -> Fade Camera And Wait(from black).
 */
UCLASS()
class SPM_GRUPP3_API UCutsceneFadeAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Fires once the fade has finished (after Duration seconds).
	UPROPERTY(BlueprintAssignable)
	FCutsceneFadeFinished OnFinished;

	// Fade the player camera to black (bToBlack = true) or back from black (bToBlack = false) over
	// Duration seconds, then continue. No separate Delay needed -- this node waits for the fade.
	UFUNCTION(BlueprintCallable, Category = "Cutscene|Fade", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static UCutsceneFadeAction* FadeCameraAndWait(const UObject* WorldContextObject, bool bToBlack, float Duration = 0.4f);

	virtual void Activate() override;

private:
	TWeakObjectPtr<UWorld> WorldPtr;
	bool bToBlack = false;
	float Duration = 0.4f;
	FTimerHandle TimerHandle;

	UFUNCTION()
	void HandleFadeComplete();
};
