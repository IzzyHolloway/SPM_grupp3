  #include "CoatPickup.h"

#include "DialogueManager.h"
#include "WardrobeComponent.h"

#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

void ACoatPickup::Interact()
{
    const bool bUnlocked = TryUnlockInWardrobe();

    if (!bUnlocked && bRequireUnlockSuccess)
    {
        if (ADialogueManager* DialogueManager = Cast<ADialogueManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())))
        {
            if (!WardrobeFailedMessage.IsEmpty())
            {
                DialogueManager->ShowMessage(WardrobeFailedMessage);
            }
        }
        return;
    }

    if (ADialogueManager* DialogueManager = Cast<ADialogueManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())))
    {
        if (!PickupMessage.IsEmpty())
        {
            DialogueManager->ShowMessage(PickupMessage);
        }
    }

    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            PickupSound,
            GetActorLocation(),
            PickupSoundVolume
        );
    }

    Destroy();
}

bool ACoatPickup::TryUnlockInWardrobe() const
{
    if (CoatID.IsNone() || CoatID == TEXT("None"))
    {
        UE_LOG(LogTemp, Warning, TEXT("Coat pickup failed: invalid CoatID."));
        return false;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Coat pickup failed: no player pawn."));
        return false;
    }

    UWardrobeComponent* WardrobeComp = PlayerPawn->FindComponentByClass<UWardrobeComponent>();
    if (!WardrobeComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Coat pickup failed: no UWardrobeComponent on player."));
        return false;
    }

    const bool bSuccess = WardrobeComp->UnlockCoat(CoatID);
    UE_LOG(LogTemp, Warning, TEXT("Tried to unlock coat: %s. Success: %s"),
        *CoatID.ToString(),
        bSuccess ? TEXT("true") : TEXT("false"));

    return bSuccess;
}
