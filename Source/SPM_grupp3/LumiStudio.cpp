#include "LumiStudio.h"
#include "WardrobeDataTypes.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"

ALumiStudio::ALumiStudio()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    // Lumi stands at the origin facing +X. Animation ticks even though she isn't in the main view,
    // but only while the studio is active (we gate the component tick in SetStudioActive).
    PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
    PreviewMesh->SetupAttachment(SceneRoot);
    PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PreviewMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    PreviewMesh->bComponentUseFixedSkelBounds = true; // avoid bounds recompute culling it out of the capture
    // The idle plays even though Lumi is off-screen (only seen via the capture). URO throttles the
    // animation rate for distant/off-screen meshes, which freezes the idle here -- turn it off.
    PreviewMesh->bEnableUpdateRateOptimizations = false;

    KeyLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("KeyLight"));
    KeyLight->SetupAttachment(SceneRoot);
    KeyLight->SetRelativeLocation(FVector(140.f, -90.f, 170.f));
    KeyLight->SetIntensity(40000.f);
    KeyLight->SetAttenuationRadius(1500.f);
    KeyLight->SetCastShadows(true);
    KeyLight->SetMobility(EComponentMobility::Movable);

    FillLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FillLight"));
    FillLight->SetupAttachment(SceneRoot);
    FillLight->SetRelativeLocation(FVector(120.f, 110.f, 90.f));
    FillLight->SetIntensity(15000.f);
    FillLight->SetAttenuationRadius(1500.f);
    FillLight->SetCastShadows(false);
    FillLight->SetMobility(EComponentMobility::Movable);

    Capture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Capture"));
    Capture->SetupAttachment(SceneRoot);
    Capture->SetRelativeLocation(FVector(CaptureDistance, 0.f, CaptureHeight));
    Capture->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
    Capture->ProjectionType = ECameraProjectionMode::Perspective;
    Capture->FOVAngle = CaptureFOV;

    // FinalColorLDR gives the lit, tonemapped image a UI material can sample directly.
    Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

    // Only render the preview mesh -- not the surrounding world. We add PreviewMesh in BeginPlay.
    Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

    // Trim expensive render passes the character preview doesn't need, so capturing every frame
    // stays cheap. (Geometry is already tiny -- only Lumi + backdrop render via the ShowOnly list.)
    Capture->ShowFlags.SetMotionBlur(false);
    Capture->ShowFlags.SetScreenSpaceReflections(false);
    Capture->ShowFlags.SetAmbientOcclusion(false);
    Capture->ShowFlags.SetBloom(false);
    Capture->ShowFlags.SetLensFlares(false);
    Capture->ShowFlags.SetSceneColorFringe(false);

    // Start idle; SetStudioActive(true) flips this on when the wardrobe opens.
    Capture->bCaptureEveryFrame = false;
    Capture->bCaptureOnMovement = false;

    // Lock exposure so the capture is lit consistently in EVERY level, regardless of any
    // PostProcessVolume the surrounding level uses. We must override the exposure METHOD too:
    // Min/MaxBrightness only apply to auto-exposure, so if a level's PPV sets Manual exposure our
    // clamp would be ignored and the capture renders black. Forcing Histogram + Min==Max pins the
    // exposure multiplier to a constant no matter what the world does.
    Capture->PostProcessSettings.bOverride_AutoExposureMethod = true;
    Capture->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
    Capture->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
    Capture->PostProcessSettings.AutoExposureMinBrightness = 1.f;
    Capture->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
    Capture->PostProcessSettings.AutoExposureMaxBrightness = 1.f;
    Capture->PostProcessSettings.bOverride_AutoExposureBias = true;
    Capture->PostProcessSettings.AutoExposureBias = 1.f;

    // Also fully apply the capture's own post-process (so a level's unbound PPV can't dilute it).
    Capture->PostProcessBlendWeight = 1.f;
}

void ALumiStudio::BeginPlay()
{
    Super::BeginPlay();

    // Re-apply framing values in case they were tweaked in the BP defaults after construction.
    Capture->SetRelativeLocation(FVector(CaptureDistance, 0.f, CaptureHeight));
    Capture->FOVAngle = CaptureFOV;

    
    Capture->ShowOnlyComponents.Empty();
    TArray<UPrimitiveComponent*> Primitives;
    GetComponents<UPrimitiveComponent>(Primitives);
    for (UPrimitiveComponent* Primitive : Primitives)
    {
        Capture->ShowOnlyComponents.Add(Primitive);
    }

    if (RenderTarget)
    {
        Capture->TextureTarget = RenderTarget;
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ALumiStudio '%s' has no RenderTarget assigned -- the wardrobe preview will be blank."),
            *GetName());
    }

    // A capture in ShowOnlyList mode renders pure black if its one mesh has no asset, so warn loudly.
    if (!PreviewMesh->GetSkeletalMeshAsset())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ALumiStudio '%s' PreviewMesh has no Skeletal Mesh assigned -- the capture will be black."),
            *GetName());
    }

    SetStudioActive(false);
}

void ALumiStudio::SetStudioActive(bool bActive)
{
    if (!Capture) return;

    UE_LOG(LogTemp, Warning, TEXT("ALumiStudio '%s' SetStudioActive(%s). RenderTarget=%s, Mesh=%s"),
        *GetName(),
        bActive ? TEXT("true") : TEXT("false"),
        RenderTarget ? TEXT("set") : TEXT("NULL"),
        PreviewMesh && PreviewMesh->GetSkeletalMeshAsset() ? TEXT("set") : TEXT("NULL"));

    // Pause the skeletal animation while hidden so we don't pay for it when the wardrobe is closed.
    PreviewMesh->SetComponentTickEnabled(bActive);

    // bCaptureEveryFrame already re-renders on the next frame, so don't also call CaptureScene()
    Capture->bCaptureEveryFrame = bActive;
}

void ALumiStudio::SetPreviewCoatMaterial(UMaterialInterface* Material, int32 MaterialSlot)
{
    if (!PreviewMesh || !Material) return;
    PreviewMesh->SetMaterial(MaterialSlot, Material);
}

void ALumiStudio::ApplyCoat(FName CoatID)
{
    if (CoatID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("ALumiStudio::ApplyCoat: CoatID is None, ignoring."));
        return;
    }
    if (!CoatDataTable)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ALumiStudio::ApplyCoat: CoatDataTable is NULL -- assign DT_Coats on BP_LumiStudio. Preview won't change coat."));
        return;
    }

    const FCoatDetail* Row = CoatDataTable->FindRow<FCoatDetail>(CoatID, TEXT("LumiStudio ApplyCoat"));
    if (!Row)
    {
        UE_LOG(LogTemp, Warning, TEXT("ALumiStudio::ApplyCoat: no row '%s' in CoatDataTable."), *CoatID.ToString());
        return;
    }
    if (!Row->CoatMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("ALumiStudio::ApplyCoat: row '%s' has no CoatMaterial."), *CoatID.ToString());
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("ALumiStudio::ApplyCoat: '%s' -> material '%s' on slot %d"),
        *CoatID.ToString(), *Row->CoatMaterial->GetName(), PreviewCoatSlot);

    // Use the studio's PreviewCoatSlot, not the DataTable's CoatMaterialSlot -- the gameplay
    // character applies coats to element 0 and that's the slot that actually shows on this mesh.
    SetPreviewCoatMaterial(Row->CoatMaterial, PreviewCoatSlot);
}
