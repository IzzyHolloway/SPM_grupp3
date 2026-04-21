// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerPawn_Izzy.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

APlayerPawn_Izzy::APlayerPawn_Izzy()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeAsset.Succeeded())
	{
		VisualMesh->SetStaticMesh(CubeAsset.Object);
	}

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->SetRelativeLocation(FVector(0.f, 1600.f, 200.f));
	PlayerCamera->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	PlayerCamera->ProjectionMode = ECameraProjectionMode::Orthographic;
	PlayerCamera->OrthoWidth = 3200.f;
}

void APlayerPawn_Izzy::BeginPlay()
{
	Super::BeginPlay();

	// Lägg till IMC i spelarens input subsystem
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
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
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerPawn_Izzy::OnMove);
			EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &APlayerPawn_Izzy::OnMove);
		}
		if (JumpAction)
		{
			EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerPawn_Izzy::OnJump);
		}
	}
}

void APlayerPawn_Izzy::OnMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	// 2D side-scroller: bara horisontell (X). Vertikal (Y) används inte.
	CurrentInput = FVector(Axis.X, 0.f, 0.f);
}

void APlayerPawn_Izzy::OnJump(const FInputActionValue& Value)
{
	if (IsGrounded())
	{
		Velocity.Z = JumpStrength;
	}
}

void APlayerPawn_Izzy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. Gravitation
	Velocity.Z += GetWorld()->GetGravityZ() * DeltaTime;

	// 2. Input-baserad acceleration
	Velocity += CurrentInput.GetSafeNormal() * Acceleration * DeltaTime;

	// 3. Luftmotstånd (exponentiell decay)
	Velocity.X *= FMath::Pow(AirResistance, DeltaTime);

	// 4. Lös kollisioner och uppdatera hastighet
	ResolveCollisions(DeltaTime);

	// 5. Flytta karaktären
	SetActorLocation(GetActorLocation() + Velocity * DeltaTime);

	// 6. Hantera överlappningar (safety net)
	Depenetrate();
}

void APlayerPawn_Izzy::ResolveCollisions(float DeltaTime)
{
	int32 Counter = 0;
	bool bHit = false;

	do
	{
		if (Velocity.SizeSquared() < 0.01f)
		{
			Velocity = FVector::ZeroVector;
			return;
		}

		if (Counter++ >= 5)
		{
			Velocity = FVector::ZeroVector;
			return;
		}

		FVector Origin, Extent;
		GetActorBounds(true, Origin, Extent);

		const FVector Movement = Velocity * DeltaTime;
		const FVector TraceStart = Origin;
		const FVector TraceEnd = Origin + Movement.GetSafeNormal() * (Movement.Size() + SkinWidth);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		FHitResult Hit;
		bHit = GetWorld()->SweepSingleByChannel(
			Hit, TraceStart, TraceEnd, FQuat::Identity,
			TraceChannel, FCollisionShape::MakeBox(Extent), Params);

		if (bHit)
		{
			const FVector NormalForce = CalculateNormalForce(Velocity, Hit.Normal);
			Velocity += NormalForce;
			ApplyFriction(NormalForce.Size());
		}
	} while (bHit);
}

FVector APlayerPawn_Izzy::CalculateNormalForce(FVector Force, FVector Normal) const
{
	const float Dot = FVector::DotProduct(Force, Normal);
0	if (Dot >= 0.f)
	{
		return FVector::ZeroVector;
	}
	const FVector Projection = Dot * Normal;
	return -Projection;
}

void APlayerPawn_Izzy::ApplyFriction(float NormalForceMagnitude)
{
	if (Velocity.Size() < NormalForceMagnitude * StaticFriction)
	{
		Velocity = FVector::ZeroVector;
	}
	else
	{
		Velocity -= Velocity.GetSafeNormal() * NormalForceMagnitude * KineticFriction;
	}
}

void APlayerPawn_Izzy::Depenetrate()
{
	FVector Origin, Extent;
	GetActorBounds(true, Origin, Extent);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	TArray<FOverlapResult> Overlaps;
	if (GetWorld()->OverlapMultiByChannel(
		Overlaps, Origin, FQuat::Identity, TraceChannel,
		FCollisionShape::MakeBox(Extent), Params))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (!Overlap.Component.IsValid()) continue;

			FMTDResult MTD;
			if (Overlap.Component->ComputePenetration(
				MTD, FCollisionShape::MakeBox(Extent + FVector(SkinWidth)),
				Origin, FQuat::Identity))
			{
				SetActorLocation(GetActorLocation() + MTD.Direction * (MTD.Distance + SkinWidth));
				Velocity += CalculateNormalForce(Velocity, -MTD.Direction);
				break; // Lös en i taget
			}
		}
	}
}

bool APlayerPawn_Izzy::IsGrounded() const
{
	FVector Origin, Extent;
	GetActorBounds(true, Origin, Extent);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	return GetWorld()->SweepSingleByChannel(
		Hit, Origin, Origin + FVector::DownVector * (GroundCheckDistance + SkinWidth),
		FQuat::Identity, TraceChannel,
		FCollisionShape::MakeBox(Extent), Params);
}