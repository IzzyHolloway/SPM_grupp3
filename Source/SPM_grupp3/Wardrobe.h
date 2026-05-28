#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "Wardrobe.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UWardrobeViewWidget;
class UWardrobeComponent;
class UInputMappingContext;
class UInputAction;
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
