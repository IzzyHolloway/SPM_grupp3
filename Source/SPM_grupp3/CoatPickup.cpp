#include "CoatPickup.h"
#include "WardrobeComponent.h"
#include "OutlineComponent.h"
#include "LittleLost_GameInstance.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ACoatPickup::ACoatPickup()
{
    OutlineComponent = CreateDefaultSubobject<UOutlineComponent>(TEXT("Outline"));
    OutlineComponent->SetupAttachment(RootComponent);
}

void ACoatPickup::BeginPlay()
{
    Super::BeginPlay();

    // Level 2 is a duplicate of Level 1, so it spawns every coat pickup again -- including the
    // ones the player already collected. The GameInstance carries the saved wardrobe across the
    // level load; if this coat is already unlocked there, remove this pickup so the player only
    // sees the coats they still haven't found. On a fresh game there is no save yet, so nothing
    // is removed and every coat is collectable as normal.
    if (ULittleLost_GameInstance* GI =
            Cast<ULittleLost_GameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        if (GI->IsCoatUnlockedInSave(CoatID))
        {
            Destroy();
        }
    }
}

void ACoatPickup::Interact()
{
    const bool bUnlocked = TryUnlockInWardrobe();

    if (!bUnlocked && bRequireUnlockSuccess)
    {
        return;
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
    //Zoey start
    if (SuccessJingleSound && GetWorld())
    {
        FTimerHandle JingleTimerHandle;
        FVector SpawnLocation = GetActorLocation();

        // Vi använder en lambda i timern eftersom denna aktör (this) kommer raderas strax
        GetWorld()->GetTimerManager().SetTimer(
            JingleTimerHandle,
            [World = GetWorld(), Sound = SuccessJingleSound, SpawnLocation, Vol = SuccessJingleVolume]()
            {
                PlaySuccessJingle(World, Sound, SpawnLocation, Vol);
            },
            JingleDelay,
            false
        );
    }
    //Zoey end
    
    
    // Clean up the on-screen prompt before destroying the actor — otherwise the
    // widget stays in the viewport because the player has no reference to call
    // SetPromptVisible(false) on after we're gone.
    SetPromptVisible(false);

    Destroy();
}

bool ACoatPickup::TryUnlockInWardrobe() const
{
    if (CoatID.IsNone() || CoatID == TEXT("None"))
    {
        UE_LOG(LogTemp, Warning, TEXT("Coat pickup failed: invalid CoatID."));
        return false;
    }

    // The wardrobe lives on the player character. Usually that's the possessed pawn, but while the
    // player is in the boat the possessed pawn is the boat (which has no wardrobe). In that case fall
    // back to searching the world for the character that actually owns the UWardrobeComponent -- it
    // still exists, attached to the boat.
    UWardrobeComponent* WardrobeComp = nullptr;

    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        WardrobeComp = PlayerPawn->FindComponentByClass<UWardrobeComponent>();
    }

    if (!WardrobeComp)
    {
        for (TActorIterator<AActor> It(GetWorld()); It; ++It)
        {
            if (UWardrobeComponent* Found = It->FindComponentByClass<UWardrobeComponent>())
            {
                WardrobeComp = Found;
                break;
            }
        }
    }

    if (!WardrobeComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Coat pickup failed: no UWardrobeComponent found in the world."));
        return false;
    }

    const bool bSuccess = WardrobeComp->UnlockCoat(CoatID);
    UE_LOG(LogTemp, Warning, TEXT("Tried to unlock coat: %s. Success: %s"),
        *CoatID.ToString(),
        bSuccess ? TEXT("true") : TEXT("false"));

    return bSuccess;
  
}
  //Zoey start
    void ACoatPickup::PlaySuccessJingle(UWorld* World, USoundBase* Sound, FVector Location, float Volume)
    {
        if (World && Sound)
        {
            UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, Volume);
        }
    }
    //Zoey end