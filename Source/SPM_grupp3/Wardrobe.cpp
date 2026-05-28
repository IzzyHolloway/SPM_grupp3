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

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"

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

    ActiveWardrobe->ActiveWardrobeActor = this;
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

    // --- Enhanced Input: push wardrobe IMC + bind actions so they only fire while open ---
    if (ULocalPlayer* LP = PC->GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (WardrobeMappingContext)
            {
                Subsystem->AddMappingContext(WardrobeMappingContext, WardrobeMappingPriority);
            }
        }
    }

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
    {
        if (NavigateAction)
        {
            NavigateBindingHandle = EIC->BindAction(
                NavigateAction, ETriggerEvent::Started, this, &AWardrobe::HandleNavigateInput).GetHandle();
        }
        if (EquipAction)
        {
            EquipBindingHandle = EIC->BindAction(
                EquipAction, ETriggerEvent::Started, this, &AWardrobe::HandleEquipInput).GetHandle();
        }
        if (CloseAction)
        {
            CloseBindingHandle = EIC->BindAction(
                CloseAction, ETriggerEvent::Started, this, &AWardrobe::HandleCloseInput).GetHandle();
        }
    }
}

void AWardrobe::CloseWardrobe()
{
    if (WardrobeViewWidget)
    {
        WardrobeViewWidget->RemoveFromParent();
        WardrobeViewWidget = nullptr;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

    // --- Enhanced Input: unbind + pop IMC ---
    if (PC)
    {
        if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
        {
            if (NavigateBindingHandle != 0) { EIC->RemoveBindingByHandle(NavigateBindingHandle); }
            if (EquipBindingHandle    != 0) { EIC->RemoveBindingByHandle(EquipBindingHandle); }
            if (CloseBindingHandle    != 0) { EIC->RemoveBindingByHandle(CloseBindingHandle); }
        }

        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                if (WardrobeMappingContext)
                {
                    Subsystem->RemoveMappingContext(WardrobeMappingContext);
                }
            }
        }
    }

    NavigateBindingHandle = 0;
    EquipBindingHandle = 0;
    CloseBindingHandle = 0;

    if (ActiveWardrobe)
    {
        ActiveWardrobe->SetWardrobeOpen(false);
        if (ActiveWardrobe->ActiveWardrobeActor.Get() == this)
        {
            ActiveWardrobe->ActiveWardrobeActor = nullptr;
        }
    }

    if (PC)
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

void AWardrobe::HandleNavigateInput(const FInputActionValue& Value)
{
    if (!ActiveWardrobe) return;

    const FVector2D Axis = Value.Get<FVector2D>();
    const int32 DeltaX = FMath::TruncToInt(Axis.X);
    // Invert Y so up on the stick moves "up" in the grid (lower row index).
    const int32 DeltaY = -FMath::TruncToInt(Axis.Y);

    if (DeltaX != 0 || DeltaY != 0)
    {
        ActiveWardrobe->MoveSelectionGrid(DeltaX, DeltaY);
    }
}

void AWardrobe::HandleEquipInput(const FInputActionValue&)
{
    if (ActiveWardrobe)
    {
        ActiveWardrobe->EquipSelected();
    }
}

void AWardrobe::HandleCloseInput(const FInputActionValue&)
{
    CloseWardrobe();
}
