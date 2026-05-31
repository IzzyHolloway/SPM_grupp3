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

    UE_LOG(LogTemp, Warning, TEXT("[Outline] SetOutline(%s) on %s -- found %d mesh(es)"),
           bEnabled ? TEXT("ON") : TEXT("off"), *Owner->GetName(), Meshes.Num());

    for (UMeshComponent* MeshComp : Meshes)
    {
        if (!MeshComp) continue;
        MeshComp->SetCustomDepthStencilValue(OutlineStencilValue);
        MeshComp->SetRenderCustomDepth(bEnabled);
        UE_LOG(LogTemp, Warning, TEXT("[Outline]   -> %s (%s), stencil=%d"),
               *MeshComp->GetName(), *MeshComp->GetClass()->GetName(), OutlineStencilValue);
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

    UE_LOG(LogTemp, Warning, TEXT("[Outline] Unsuppress on %s -- player=%s, stillInside=%s"),
           GetOwner() ? *GetOwner()->GetName() : TEXT("?"),
           *GetNameSafe(PlayerPawn), bPlayerInside ? TEXT("YES") : TEXT("NO"));

    SetOutline(bPlayerInside);
}

void UOutlineComponent::HandleBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
                                           UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    UE_LOG(LogTemp, Warning, TEXT("[Outline] BeginOverlap on %s by %s (player=%s)"),
           GetOwner() ? *GetOwner()->GetName() : TEXT("?"),
           OtherActor ? *OtherActor->GetName() : TEXT("null"),
           *GetNameSafe(UGameplayStatics::GetPlayerPawn(this, 0)));

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
