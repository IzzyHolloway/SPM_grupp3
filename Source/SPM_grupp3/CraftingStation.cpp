#include "CraftingStation.h"
#include "CraftingViewWidget.h"
#include "InventoryComponent.h"
#include "CharacterAimi.h"
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
    SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ACraftingStation::OnSphereBeginOverlap);

    // Outline starts off; the stencil value just needs to match the M_outline post-process material.
    if (Mesh)
    {
        Mesh->SetCustomDepthStencilValue(OutlineStencilValue);
        Mesh->SetRenderCustomDepth(false);
    }
}

void ACraftingStation::OnSphereBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
                                            UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    // Only the player should light up the outline.
    if (Cast<ACharacterAimi>(OtherActor))
    {
        SetOutlineEnabled(true);
    }
}

void ACraftingStation::SetOutlineEnabled(bool bEnabled)
{
    if (!bShowOutlineWhenNear || !Mesh) return;

    // While crafting is open we never glow, even though the player is standing inside the sphere.
    Mesh->SetRenderCustomDepth(bEnabled && !bOutlineSuppressed);
}

FText ACraftingStation::LookAtActor_Implementation() const
{
    return StationName;
}

void ACraftingStation::InteractWith_Implementation(AActor* Interactor)
{
   
    if (!IsCraftingOpen())
    {
        OpenCrafting(Interactor);
    }
}

void ACraftingStation::OpenCrafting(AActor* Interactor)
{
    if (!Interactor || !CraftingViewClass) return;

    ActiveInventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (!ActiveInventory) return;

    if (!ActiveInventory->OnCraftSuccess.IsAlreadyBound(this, &ACraftingStation::HandleCraftSuccess))
    {
        ActiveInventory->OnCraftSuccess.AddDynamic(this, &ACraftingStation::HandleCraftSuccess);
    }

    if (!ActiveInventory->OnPuzzleCraftRequested.IsAlreadyBound(this, &ACraftingStation::HandlePuzzleCraftRequested))
    {
        ActiveInventory->OnPuzzleCraftRequested.AddDynamic(this, &ACraftingStation::HandlePuzzleCraftRequested);
    }

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

    // Don't keep the workbench outlined the whole time it's open.
    bOutlineSuppressed = true;
    SetOutlineEnabled(false);

    if (ACharacter* Character = Cast<ACharacter>(Interactor))
    {
        Character->GetCharacterMovement()->DisableMovement();
    }

    // Stop the player from running interactable detection while crafting is open.
    // Without this the "Press E" prompt re-spawns every tick on top of the crafting UI.
    if (ACharacterAimi* Player = Cast<ACharacterAimi>(Interactor))
    {
        Player->SetInteractionDetectionEnabled(false);
    }

    UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(
        PC, nullptr, EMouseLockMode::DoNotLock, true);
    PC->bShowMouseCursor = false;
}

void ACraftingStation::CloseCrafting()
{
    if (CraftingViewWidget)
    {
        CraftingViewWidget->RemoveFromParent();
        CraftingViewWidget = nullptr;
    }

    // Release the suppression and restore the real state: only glow again if the player is
    // actually still standing in range (they may have walked away, which is what closed it).
    bOutlineSuppressed = false;
    const bool bPlayerStillNear =
        ActiveInteractor.IsValid() && SphereCollision &&
        SphereCollision->IsOverlappingActor(ActiveInteractor.Get());
    SetOutlineEnabled(bPlayerStillNear);

    if (ActiveInventory)
    {
        ActiveInventory->OnCraftSuccess.RemoveDynamic(this, &ACraftingStation::HandleCraftSuccess);
        ActiveInventory->OnPuzzleCraftRequested.RemoveDynamic(this, &ACraftingStation::HandlePuzzleCraftRequested); //Izzy lagt till för ritpussel
        ActiveInventory->ClearWorkbench();
        ActiveInventory->SetWorkbenchOpen(false);
    }

    GetWorld()->GetTimerManager().ClearTimer(CloseAfterCraftTimer);

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

        // Re-enable interactable detection so prompts work again after crafting closes.
        if (ACharacterAimi* Player = Cast<ACharacterAimi>(ActiveInteractor.Get()))
        {
            Player->SetInteractionDetectionEnabled(true);
        }
    }

    ActiveInventory = nullptr;
    ActiveInteractor = nullptr;
}

void ACraftingStation::OnSphereEndOverlap(UPrimitiveComponent* ,
                                          AActor* OtherActor,
                                          UPrimitiveComponent* ,
                                          int32)
{
    // Turn the outline off when the player leaves (runs even if crafting isn't open).
    if (Cast<ACharacterAimi>(OtherActor))
    {
        SetOutlineEnabled(false);
    }

    if (!IsCraftingOpen()) return;
    if (OtherActor == ActiveInteractor.Get())
    {
        CloseCrafting();
    }
}

void ACraftingStation::HandleCraftSuccess()
{
    if (!IsCraftingOpen()) return;

    GetWorld()->GetTimerManager().SetTimer(
        CloseAfterCraftTimer, this, &ACraftingStation::CloseCrafting,
        FMath::Max(0.f, CloseDelayAfterCraft), false);
}

void ACraftingStation::HandlePuzzleCraftRequested(FName ,
                                                  TSubclassOf<UUserWidget> )
{
    if (!IsCraftingOpen()) return;
    
    GetWorld()->GetTimerManager().ClearTimer(CloseAfterCraftTimer);
    CloseCrafting();
}
