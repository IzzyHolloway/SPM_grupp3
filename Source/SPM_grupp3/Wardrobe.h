#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "Sound/SoundBase.h" //Zoeys
#include "Wardrobe.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UWardrobeViewWidget;
class UWardrobeComponent;
class UInputMappingContext;
class UInputAction;
class ALumiStudio;
struct FInputActionValue;

UCLASS()
class SPM_GRUPP3_API AWardrobe : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AWardrobe();

    // IInteractable
    virtual void InteractWith_Implementation(AActor* Interactor) override;
    virtual FText LookAtActor_Implementation() const override;

    // Outline is enough for the wardrobe -- don't show the "X Interact" prompt.
    virtual bool ShouldShowInteractPrompt_Implementation() const override { return false; }

    UFUNCTION(BlueprintPure, Category = "Wardrobe")
    bool IsWardrobeOpen() const { return WardrobeViewWidget != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Wardrobe")
    void OpenWardrobe(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Wardrobe")
    void CloseWardrobe();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Enhanced Input handlers. Pushed/popped via the wardrobe IMC so they only
    // fire while the wardrobe is open — no conflict with inventory bindings.
    void HandleNavigateInput(const FInputActionValue& Value);
    void HandleEquipInput(const FInputActionValue& Value);
    void HandleCloseInput(const FInputActionValue& Value);

    // Forwards the wardrobe component's OnEquippedCoatChanged into the BP event below.
    UFUNCTION()
    void HandleEquippedCoatChanged(FName NewCoatID);

public:
    // Override this in BP_Wardrobe to react to an equip — typically by looking up
    // the CoatMaterial in DT_Coats and applying it to Wearer's mesh.
    // Wearer is the actor that opened the wardrobe (the player character).
    UFUNCTION(BlueprintImplementableEvent, Category = "Wardrobe")
    void OnCoatEquipped(FName CoatID, AActor* Wearer);
    
    //Zoey start
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wardrobe|Audio")
    TObjectPtr<USoundBase> EquipSound;
    //Zoey end

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> SphereCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe")
    FText StationName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wardrobe")
    TSubclassOf<UWardrobeViewWidget> WardrobeViewClass;

    // IMC pushed onto the player's EnhancedInput subsystem while the wardrobe is open.
    // Should map keys to NavigateAction / EquipAction / CloseAction below.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wardrobe|Input")
    TObjectPtr<UInputMappingContext> WardrobeMappingContext;

    // Higher than your gameplay IMC priority so wardrobe input wins while open.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wardrobe|Input")
    int32 WardrobeMappingPriority = 10;

    // Axis2D action (D-pad / arrows / thumbstick). Calls Wardrobe->MoveSelectionGrid.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wardrobe|Input")
    TObjectPtr<UInputAction> NavigateAction;

    // Digital action (A / Enter / LMB). Calls Wardrobe->EquipSelected.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wardrobe|Input")
    TObjectPtr<UInputAction> EquipAction;

    // Digital action (B / Esc). Calls CloseWardrobe.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wardrobe|Input")
    TObjectPtr<UInputAction> CloseAction;

    // The Lumi preview studio. Optional: drag a placed BP_LumiStudio actor here. If left empty, the
    // wardrobe auto-finds one in the level, and if none exists it spawns one from PreviewStudioClass.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Preview")
    TObjectPtr<ALumiStudio> PreviewStudio;

    // Studio class to spawn if no ALumiStudio exists in the played level. Set this to BP_LumiStudio
    // in the editor (a class reference works in BP defaults, so no per-level placement is needed).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Preview")
    TSubclassOf<ALumiStudio> PreviewStudioClass;

protected:
    UPROPERTY(Transient)
    TObjectPtr<UWardrobeViewWidget> WardrobeViewWidget;

    UPROPERTY(Transient)
    TObjectPtr<UWardrobeComponent> ActiveWardrobe;

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> ActiveInteractor;

    // Cached binding handles so CloseWardrobe can unbind cleanly.
    uint32 NavigateBindingHandle = 0;
    uint32 EquipBindingHandle = 0;
    uint32 CloseBindingHandle = 0;
};
