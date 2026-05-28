#include "Wardrobe.h"
#include "WardrobeViewWidget.h"
#include "WardrobeComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AWardrobe::AWardrobe()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);

    SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
    SphereCollision->SetupAttachment(Mesh);
    SphereCollision->SetSphereRadius(250.f);
    SphereCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AWardrobe::BeginPlay()
{
    Super::BeginPlay();
    SphereCollision->OnComponentEndOverlap.AddDynamic(this, &AWardrobe::OnSphereEndOverlap);
}

FText AWardrobe::LookAtActor_Implementation() const
{
    return StationName;
}

void AWardrobe::InteractWith_Implementation(AActor* Interactor)
{
    if (!IsWardrobeOpen())
    {
        OpenWardrobe(Interactor);
    }
}

void AWardrobe::OpenWardrobe(AActor* Interactor)
{
    if (!Interactor || !WardrobeViewClass) return;

    ActiveWardrobe = Interactor->FindComponentByClass<UWardrobeComponent>();
    if (!ActiveWardrobe) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;

    WardrobeViewWidget = CreateWidget<UWardrobeViewWidget>(PC, WardrobeViewClass);
    if (!WardrobeViewWidget) return;

    WardrobeViewWidget->Wardrobe = ActiveWardrobe;
    WardrobeViewWidget->WardrobeActor = this;
    WardrobeViewWidget->AddToViewport();

    ActiveWardrobe->SetWardrobeOpen(true);
    // Start with the cursor on the currently equipped coat so the "Selected" frame
    // sits where the player expects it on first open.
    ActiveWardrobe->SelectEquippedSlot();

    ActiveInteractor = Interactor;

    if (ACharacter* Character = Cast<ACharacter>(Interactor))
    {
        Character->GetCharacterMovement()->DisableMovement();
    }

    // Mouse-driven navigation per the user flow (PC: hover + LMB).
    UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(
        PC, nullptr, EMouseLockMode::DoNotLock, true);
    PC->bShowMouseCursor = true;
}

void AWardrobe::CloseWardrobe()
{
    if (WardrobeViewWidget)
    {
        WardrobeViewWidget->RemoveFromParent();
        WardrobeViewWidget = nullptr;
    }

    if (ActiveWardrobe)
    {
        ActiveWardrobe->SetWardrobeOpen(false);
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
        PC->bShowMouseCursor = false;
    }

    if (ActiveInteractor.IsValid())
    {
        if (ACharacter* Character = Cast<ACharacter>(ActiveInteractor.Get()))
        {
            Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }

    ActiveWardrobe = nullptr;
    ActiveInteractor = nullptr;
}

void AWardrobe::OnSphereEndOverlap(UPrimitiveComponent*,
                                   AActor* OtherActor,
                                   UPrimitiveComponent*,
                                   int32)
{
    if (!IsWardrobeOpen()) return;
    if (OtherActor == ActiveInteractor.Get())
    {
        CloseWardrobe();
    }
}
