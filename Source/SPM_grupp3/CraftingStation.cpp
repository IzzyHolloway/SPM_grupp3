#include "CraftingStation.h"
#include "CraftingViewWidget.h"
#include "InventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PanelWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACraftingStation::ACraftingStation()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);

    SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
    SphereCollision->SetupAttachment(Mesh);
    SphereCollision->SetSphereRadius(250.f);
    SphereCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ACraftingStation::BeginPlay()
{
    Super::BeginPlay();
    SphereCollision->OnComponentEndOverlap.AddDynamic(this, &ACraftingStation::OnSphereEndOverlap);
}

FText ACraftingStation::LookAtActor_Implementation() const
{
    return StationName;
}

void ACraftingStation::InteractWith_Implementation(AActor* Interactor)
{
    if (IsCraftingOpen())
    {
        CloseCrafting();
    }
    else
    {
        OpenCrafting(Interactor);
    }
}

void ACraftingStation::OpenCrafting(AActor* Interactor)
{
    if (!Interactor || !CraftingViewClass) return;

    ActiveInventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (!ActiveInventory) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;

    CraftingViewWidget = CreateWidget<UCraftingViewWidget>(PC, CraftingViewClass);
    if (!CraftingViewWidget) return;

    CraftingViewWidget->CraftingInventory = ActiveInventory;
    CraftingViewWidget->CraftingStation   = this;
    CraftingViewWidget->AddToViewport();

    ActiveInventory->ClearWorkbench();
    ActiveInventory->SetWorkbenchOpen(true);
    ActiveInventory->SelectFirstAvailableSlot();

    ActiveInteractor = Interactor;

    if (ACharacter* Character = Cast<ACharacter>(Interactor))
    {
        Character->GetCharacterMovement()->DisableMovement();
    }

    UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(
        PC, nullptr, EMouseLockMode::DoNotLock, /*bHideCursorDuringCapture=*/true);
    PC->bShowMouseCursor = true;
}

void ACraftingStation::CloseCrafting()
{
    if (CraftingViewWidget)
    {
        CraftingViewWidget->RemoveFromParent();
        CraftingViewWidget = nullptr;
    }

    if (ActiveInventory)
    {
        ActiveInventory->ClearWorkbench();
        ActiveInventory->SetWorkbenchOpen(false);
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

    ActiveInventory = nullptr;
    ActiveInteractor = nullptr;
}

void ACraftingStation::OnSphereEndOverlap(UPrimitiveComponent* /*OverlappedComponent*/,
                                          AActor* OtherActor,
                                          UPrimitiveComponent* /*OtherComp*/,
                                          int32 /*OtherBodyIndex*/)
{
    if (!IsCraftingOpen()) return;
    if (OtherActor == ActiveInteractor.Get())
    {
        CloseCrafting();
    }
}
