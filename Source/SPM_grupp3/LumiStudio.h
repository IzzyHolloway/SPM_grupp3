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

    // Optional DT_Coats. If set, ApplyCoat(CoatID) reads CoatMaterial from the matching FCoatDetail
    // row and applies it. Leave null and use SetPreviewCoatMaterial instead.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumi Studio")
    TObjectPtr<UDataTable> CoatDataTable;

    // Which material element on the preview mesh the coat lives on. The gameplay character swaps the
    // coat on element 0, so default to 0 here too (the DataTable's per-coat CoatMaterialSlot is the
    // wrong slot for this mesh). Change this only if your preview mesh puts the coat on another slot.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumi Studio")
    int32 PreviewCoatSlot = 0;

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
