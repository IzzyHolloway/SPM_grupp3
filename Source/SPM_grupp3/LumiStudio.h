#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LumiStudio.generated.h"

class USkeletalMeshComponent;
class USceneCaptureComponent2D;
class UPointLightComponent;
class UTextureRenderTarget2D;
class UMaterialInterface;
class UDataTable;

/**
 * Self-contained "photo studio" that renders a preview copy of Lumi into a render target so the
 * wardrobe UI can show her live (idle animation + currently equipped coat).
 *
 * Place one of these somewhere far away from the playable area (e.g. deep below the floor, or in
 * the L_GarderobStudio sublevel). It carries its own lights so it does NOT matter how dark the
 * surrounding level is -- that is the usual cause of a pure-black preview.
 *
 * Setup:
 *   1. Drop a BP child of this actor into the level. On the PreviewMesh component set Lumi's
 *      Skeletal Mesh + her idle Anim Blueprint (Anim Class).
 *   2. Assign RenderTarget = RT_LumiPreview (the same RT your RT_LumiPreview_Mat samples).
 *   3. (Optional) Assign CoatDataTable = DT_Coats so ApplyCoat(CoatID) can look materials up itself.
 *   4. In BP_Wardrobe: on OpenWardrobe -> Studio->SetStudioActive(true); on CloseWardrobe -> false.
 *   5. In BP_Wardrobe's OnCoatEquipped (or by binding OnEquippedCoatChanged) call
 *      Studio->ApplyCoat(CoatID)  (or SetPreviewCoatMaterial directly).
 */
UCLASS()
class SPM_GRUPP3_API ALumiStudio : public AActor
{
    GENERATED_BODY()

public:
    ALumiStudio();

    // Assign RT_LumiPreview here. The studio renders into this every frame while active, and your
    // widget material (RT_LumiPreview_Mat) samples the same asset -- so the UI updates live.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumi Studio")
    TObjectPtr<UTextureRenderTarget2D> RenderTarget;

    // Optional DT_Coats. If set, ApplyCoat(CoatID) reads CoatMaterial / CoatMaterialSlot from the
    // matching FCoatDetail row and applies it. Leave null and use SetPreviewCoatMaterial instead.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumi Studio")
    TObjectPtr<UDataTable> CoatDataTable;

    // Turn capturing on/off. Only renders while active, so there's no cost when the wardrobe is shut.
    // Call with true from OpenWardrobe and false from CloseWardrobe.
    UFUNCTION(BlueprintCallable, Category = "Lumi Studio")
    void SetStudioActive(bool bActive);

    // Apply a coat material directly to the preview Lumi (mirror of what BP_Wardrobe does to the player).
    UFUNCTION(BlueprintCallable, Category = "Lumi Studio")
    void SetPreviewCoatMaterial(UMaterialInterface* Material, int32 MaterialSlot = 0);

    // Look CoatID up in CoatDataTable (FCoatDetail) and apply its material. Hook this straight to the
    // wardrobe's OnEquippedCoatChanged so the preview always matches what Lumi wears.
    UFUNCTION(BlueprintCallable, Category = "Lumi Studio")
    void ApplyCoat(FName CoatID);

    UFUNCTION(BlueprintPure, Category = "Lumi Studio")
    USkeletalMeshComponent* GetPreviewMesh() const { return PreviewMesh; }

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lumi Studio")
    TObjectPtr<USceneComponent> SceneRoot;

    // The preview copy of Lumi. Set its Skeletal Mesh + idle Anim Class in the BP child's defaults.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lumi Studio")
    TObjectPtr<USkeletalMeshComponent> PreviewMesh;

    // Local key + fill lights so Lumi is always lit regardless of the level around the studio.
    // (Local lights, not directional -- a directional light would leak onto the whole game world.)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lumi Studio")
    TObjectPtr<UPointLightComponent> KeyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lumi Studio")
    TObjectPtr<UPointLightComponent> FillLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lumi Studio")
    TObjectPtr<USceneCaptureComponent2D> Capture;

    // Camera framing. Distance in front of Lumi, the height it looks at, and the lens FOV.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumi Studio|Camera")
    float CaptureDistance = 220.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumi Studio|Camera")
    float CaptureHeight = 95.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumi Studio|Camera")
    float CaptureFOV = 35.f;
};
