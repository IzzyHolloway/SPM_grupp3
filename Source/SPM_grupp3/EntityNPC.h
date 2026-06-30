// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "NPCInteractable.h"
#include "EntityNPC.generated.h"

class ADialogueManager;
class AProgressionManager;

/*
 * The Entity NPC.
 *
 * Her dialogue is started from the Level Blueprint (not the normal ANPCInteractable path),
 * so instead of hooking Interact() we listen to the DialogueManager's OnDialogueEnded and
 * watch the progression flag her conversation sets (FadeOutWhenFlagSet, default
 * "TalkedToLighthouseEntity"). Once that flag is present she fades away:
 *   - her particle effect (EntityParticles, assigned via ParticleActor) is deactivated so
 *     the already-spawned particles die out naturally,
 *   - the mesh shrinks down over FadeOutDuration (a stand-in for an opacity fade, since
 *     M_TheEntity is an opaque material),
 *   - finally both actors are removed (or just hidden, see bDestroyOnFinish).
 *
 * Set FadeOutDuration to 0 to remove her instantly instead of fading.
 */
UCLASS()
class SPM_GRUPP3_API AEntityNPC : public ANPCInteractable
{
	GENERATED_BODY()

public:
	AEntityNPC();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// The EntityParticles actor placed in the level. Assign this on the level instance so she
	// knows which particle effect to fade out alongside her.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Entity")
	TObjectPtr<AActor> ParticleActor;

	// When the player has this progression flag, the Entity fades out. This is the flag her
	// dialogue sets when it finishes (seen in the logs as "TalkedToLighthouseEntity").
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	FName FadeOutWhenFlagSet = TEXT("TalkedToLighthouseEntity");

	// Seconds to wait after the dialogue ends before she starts fading out.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	float FadeOutDelay = 1.0f;

	// Seconds the fade-out takes. Set to 0 to remove her instantly.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	float FadeOutDuration = 1.5f;

	// If true both actors are destroyed when the fade finishes; otherwise they are just hidden.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	bool bDestroyOnFinish = true;

private:
	// Called whenever any dialogue ends; fades out if the trigger flag is now set.
	UFUNCTION()
	void HandleAnyDialogueEnded();

	AProgressionManager* GetProgressionManager() const;
	bool IsTriggerFlagSet() const;

	void StartFadeOut();
	void FinishFadeOut();

	// Deactivates every FX/particle component on an actor so existing particles die out naturally.
	void DeactivateParticles(AActor* Actor);

	UPROPERTY()
	TObjectPtr<ADialogueManager> BoundManager;

	FTimerHandle FadeOutDelayTimerHandle;

	bool bHasFadedOut = false;
	bool bFading = false;
	float FadeElapsed = 0.f;
	FVector InitialScale = FVector::OneVector;
};
