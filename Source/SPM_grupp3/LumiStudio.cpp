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

    // --- Local lights. Bright by default so the preview is never black; dim them in the BP if needed. ---
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

    // --- Capture camera. Sits in front of Lumi (+X) looking back toward her (-X). ---
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

    // Start idle; SetStudioActive(true) flips this on when the wardrobe opens.
    Capture->bCaptureEveryFrame = false;
    Capture->bCaptureOnMovement = false;

    // Lock exposure so the capture can't auto-adapt itself down to black. Pinning Min == Max forces
    // the eye-adaptation multiplier to a constant regardless of the auto-exposure method, which is the
    // reliable way to get a consistently-lit scene capture (plain Manual mode often renders near-black).
    Capture->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
    Capture->PostProcessSettings.AutoExposureMinBrightness = 1.f;
    Capture->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
    Capture->PostProcessSettings.AutoExposureMaxBrightness = 1.f;
    Capture->PostProcessSettings.bOverride_AutoExposureBias = true;
    Capture->PostProcessSettings.AutoExposureBias = 1.f;
}

void ALumiStudio::BeginPlay()
{
    Super::BeginPlay();

    // Re-apply framing values in case they were tweaked in the BP defaults after construction.
    Capture->SetRelativeLocation(FVector(CaptureDistance, 0.f, CaptureHeight));
    Capture->FOVAngle = CaptureFOV;

    Capture->ShowOnlyComponents.Empty();
    Capture->ShowOnlyComponents.Add(PreviewMesh);

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

    Capture->bCaptureEveryFrame = bActive;

    if (bActive && RenderTarget)
    {
        // Render one frame immediately so the preview is filled the instant the wardrobe opens,
        // instead of showing a stale/black target for a frame.
        Capture->CaptureScene();
    }
}

void ALumiStudio::SetPreviewCoatMaterial(UMaterialInterface* Material, int32 MaterialSlot)
{
    if (!PreviewMesh || !Material) return;
    PreviewMesh->SetMaterial(MaterialSlot, Material);
}

void ALumiStudio::ApplyCoat(FName CoatID)
{
    if (CoatID.IsNone() || !CoatDataTable) return;

    const FCoatDetail* Row = CoatDataTable->FindRow<FCoatDetail>(CoatID, TEXT("LumiStudio ApplyCoat"));
    if (!Row || !Row->CoatMaterial) return;

    SetPreviewCoatMaterial(Row->CoatMaterial, Row->CoatMaterialSlot);
}
