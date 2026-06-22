#include "CutsceneWidgetBase.h"
#include "CharacterAimi.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UCutsceneWidgetBase::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Warning, TEXT("CutsceneWidgetBase: NativeConstruct on %s (bLockPlayerDuringCutscene=%d)"),
        *GetName(), bLockPlayerDuringCutscene ? 1 : 0);

    if (bLockPlayerDuringCutscene)
    {
        SetPlayerLocked(true);
    }
}

void UCutsceneWidgetBase::NativeDestruct()
{
    UE_LOG(LogTemp, Warning, TEXT("CutsceneWidgetBase: NativeDestruct on %s"), *GetName());

    // Always restore control, even if the cutscene was skipped or the widget force-removed.
    SetPlayerLocked(false);

    Super::NativeDestruct();
}

void UCutsceneWidgetBase::SetPlayerLocked(bool bLocked)
{
    APlayerController* PC = GetOwningPlayer();
    ACharacterAimi* Player = Cast<ACharacterAimi>(UGameplayStatics::GetPlayerCharacter(this, 0));

    UE_LOG(LogTemp, Warning, TEXT("CutsceneWidgetBase: SetPlayerLocked(%d)  PC=%s  Player(ACharacterAimi)=%s"),
        bLocked ? 1 : 0,
        PC ? TEXT("OK") : TEXT("NULL"),
        Player ? *Player->GetName() : TEXT("NULL"));

    if (PC)
    {
        if (bLocked)
        {
            PC->SetIgnoreMoveInput(true);
            PC->SetIgnoreLookInput(true);
        }
        else
        {
            // Reset clears the whole ignore-counter, so control is fully restored regardless of
            // how many times it was locked.
            PC->ResetIgnoreMoveInput();
            PC->ResetIgnoreLookInput();
        }
    }

    if (Player)
    {
        Player->SetMovementLocked(bLocked);
    }
}
