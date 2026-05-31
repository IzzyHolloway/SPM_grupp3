#include "OutlineComponent.h"
#include "Components/MeshComponent.h"
#include "Kismet/GameplayStatics.h"

UOutlineComponent::UOutlineComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Sensible defaults: a proximity trigger that overlaps the player. Tweak the radius per actor.
    InitSphereRadius(250.f);
    SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    SetGenerateOverlapEvents(true);
}

void UOutlineComponent::BeginPlay()
{
    Super::BeginPlay();

    OnComponentBeginOverlap.AddDynamic(this, &UOutlineComponent::HandleBeginOverlap);
    OnComponentEndOverlap.AddDynamic(this, &UOutlineComponent::HandleEndOverlap);

    // Start off (this also stamps the stencil value on the meshes).
    SetOutline(false);
}

void UOutlineComponent::SetOutline(bool bEnabled)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Outline every mesh on the owner (static + skeletal). Lights/cameras/the sphere aren't meshes.
    TArray<UMeshComponent*> Meshes;
    Owner->GetComponents<UMeshComponent>(Meshes);
    for (UMeshComponent* MeshComp : Meshes)
    {
        if (!MeshComp) continue;
        MeshComp->SetCustomDepthStencilValue(OutlineStencilValue);
        MeshComp->SetRenderCustomDepth(bEnabled);
    }
}

void UOutlineComponent::SetOutlineSuppressed(bool bSuppressed)
{
    bOutlineSuppressed = bSuppressed;

    if (bOutlineSuppressed)
    {
        SetOutline(false);
        return;
    }

    // Releasing suppression: recompute the real overlap state. We can't trust bPlayerInside here
    // because overlap-end is never fired when the player was attached to this actor (e.g. riding
    // the boat), so query the actual overlap instead.
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    bPlayerInside = (PlayerPawn != nullptr) && IsOverlappingActor(PlayerPawn);
    SetOutline(bPlayerInside);
}

void UOutlineComponent::HandleBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
                                           UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    // Only the player lights the outline.
    if (OtherActor && OtherActor == UGameplayStatics::GetPlayerPawn(this, 0))
    {
        bPlayerInside = true;
        if (!bOutlineSuppressed)
        {
            SetOutline(true);
        }
    }
}

void UOutlineComponent::HandleEndOverlap(UPrimitiveComponent*, AActor* OtherActor,
                                         UPrimitiveComponent*, int32)
{
    if (OtherActor && OtherActor == UGameplayStatics::GetPlayerPawn(this, 0))
    {
        bPlayerInside = false;
        SetOutline(false);
    }
}
