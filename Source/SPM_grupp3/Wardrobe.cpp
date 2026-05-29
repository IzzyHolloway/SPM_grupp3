#include "Wardrobe.h"
#include "WardrobeViewWidget.h"
#include "WardrobeComponent.h"
#include "CharacterAimi.h"
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

    // Listen for equip changes so BP_Wardrobe can react via OnCoatEquipped.
    if (!ActiveWardrobe->OnEquippedCoatChanged.IsAlreadyBound(this, &AWardrobe::HandleEquippedCoatChanged))
    {
        ActiveWardrobe->OnEquippedCoatChanged.AddDynamic(this, &AWardrobe::HandleEquippedCoatChanged);
    }

    ActiveWardrobe->SetWardrobeOpen(true);
    // Start with the cursor on the currently equipped coat so the "Selected" frame
    // sits where the player expects it on first open.
    ActiveWardrobe->SelectEquippedSlot();

    ActiveInteractor = Interactor;

    if (ACharacter* Character = Cast<ACharacter>(Interactor))
    {
        Character->GetCharacterMovement()->DisableMovement();
    }

    // Stop interactable detection while wardrobe is open so the "Press E" prompt
    // doesn't re-appear under the wardrobe UI every tick.
    if (ACharacterAimi* Player = Cast<ACharacterAimi>(Interactor))
    {
        Player->SetInteractionDetectionEnabled(false);
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
        ActiveWardrobe->OnEquippedCoatChanged.RemoveDynamic(this, &AWardrobe::HandleEquippedCoatChanged);
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

        // Re-enable interactable detection so prompts work again after wardrobe closes.
        if (ACharacterAimi* Player = Cast<ACharacterAimi>(ActiveInteractor.Get()))
        {
            Player->SetInteractionDetectionEnabled(true);
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
        ActiveWardrobe->MarkGamepadNavInput();
        ActiveWardrobe->MoveSelectionGrid(DeltaX, DeltaY);
    }
}

void AWardrobe::HandleEquipInput(const FInputActionValue&)
{
    UE_LOG(LogTemp, Warning, TEXT("AWardrobe::HandleEquipInput fired. SelectedSlot=%d, EquippedID=%s"),
        ActiveWardrobe ? ActiveWardrobe->SelectedSlotIndex : -1,
        ActiveWardrobe ? *ActiveWardrobe->EquippedCoatID.ToString() : TEXT("(no wardrobe)"));

    if (ActiveWardrobe)
    {
        ActiveWardrobe->EquipSelected();
    }
}

void AWardrobe::HandleCloseInput(const FInputActionValue&)
{
    CloseWardrobe();
}

void AWardrobe::HandleEquippedCoatChanged(FName NewCoatID)
{
    AActor* Wearer = ActiveInteractor.Get();
    UE_LOG(LogTemp, Warning, TEXT("AWardrobe::HandleEquippedCoatChanged: CoatID=%s, Wearer=%s"),
        *NewCoatID.ToString(),
        Wearer ? *Wearer->GetName() : TEXT("NULL"));

    // Forward to the BP-overridable event so designers can swap Lumi's material there.
    OnCoatEquipped(NewCoatID, Wearer);
}
