#include "PlayerPawn_Izzy.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

APlayerPawn_Izzy::APlayerPawn_Izzy()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Gör så att denna pawn automatiskt styrs av spelaren
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    
    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(RootComponent);

    PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
    PlayerCamera->SetupAttachment(RootComponent);
    
    // Standardposition för 2D-kamera
    PlayerCamera->SetRelativeLocation(FVector(0.f, 1600.f, 200.f));
    PlayerCamera->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    PlayerCamera->ProjectionMode = ECameraProjectionMode::Orthographic;
    PlayerCamera->OrthoWidth = 2000.f;
}

void APlayerPawn_Izzy::BeginPlay()
{
    Super::BeginPlay();

    // Lägg till din IMC_Izzy
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (InputMapping) 
            {
                Subsystem->AddMappingContext(InputMapping, 0);
            }
        }
    }
}

void APlayerPawn_Izzy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerPawn_Izzy::OnMove);
        EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &APlayerPawn_Izzy::OnMove);
        EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerPawn_Izzy::OnJump);
    }
}

void APlayerPawn_Izzy::OnMove(const FInputActionValue& Value)
{
    FVector2D MoveVec = Value.Get<FVector2D>();
    CurrentInput = FVector(MoveVec.X, 0.f, 0.f); // Endast X för 2D
}

void APlayerPawn_Izzy::OnJump(const FInputActionValue& Value)
{
    FHitResult Hit;
    FVector Start = GetActorLocation();
    FVector End = Start + FVector::DownVector * (GroundCheckDistance + SkinWidth);
    
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    // Hoppa bara om vi rör marken
    if (GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, TraceChannel, FCollisionShape::MakeBox(CollisionExtent), Params))
    {
        Velocity.Z = JumpHeight;
    }
}

void APlayerPawn_Izzy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 1. Applicera Gravitation
    Velocity.Z += GetWorld()->GetGravityZ() * DeltaTime;

    // 2. Applicera Input (Acceleration)
    Velocity += CurrentInput * Acceleration * DeltaTime;

    // 3. Luftmotstånd (Exponential)
    Velocity.X *= FMath::Pow(AirResistance, DeltaTime);

    // 4. Hantera Kollision
    CollisionFunction(DeltaTime);

    // 5. Slutgiltig förflyttning
    AddActorWorldOffset(Velocity * DeltaTime);
}

void APlayerPawn_Izzy::CollisionFunction(float DeltaTime)
{
    int32 Counter = 0;
    bool bHit = false;

    do {
        FVector Movement = Velocity * DeltaTime;
        if (Velocity.SizeSquared() < 0.001f || Counter++ > 5) return;

        FVector Start = GetActorLocation();
        // Vi kastar strålen lite längre (SkinWidth) för att upptäcka ytor vi nästan nuddar
        FVector End = Start + Movement.GetSafeNormal() * (Movement.Size() + SkinWidth);

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, TraceChannel, FCollisionShape::MakeBox(CollisionExtent), Params);

        if (bHit)
        {
            // Hur mycket av vår rörelse är tillåten innan vi nuddar ytan?
            float Dot = FVector::DotProduct(Movement.GetSafeNormal(), Hit.Normal);
            float DistToSurface = Hit.Distance - SkinWidth;
            
            if (DistToSurface > 0)
            {
                AddActorWorldOffset(Movement.GetSafeNormal() * DistToSurface);
            }

            // Applicera Normalkraft så vi glider längs ytan istället för att fastna
            Velocity += CalculateNormalForce(Velocity, Hit.Normal);
        }
    } while (bHit);
}

FVector APlayerPawn_Izzy::CalculateNormalForce(FVector Force, FVector Normal) const
{
    float Dot = FVector::DotProduct(Force, Normal);
    if (Dot >= 0.f) return FVector::ZeroVector;

    return -(Dot * Normal);
}