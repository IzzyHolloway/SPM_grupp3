#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "CraftingStation.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UCraftingViewWidget;
class UInventoryComponent;
class UUserWidget;

UCLASS()
class SPM_GRUPP3_API ACraftingStation : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    ACraftingStation();

    // IInteractable
    virtual void InteractWith_Implementation(AActor* Interactor) override;
    virtual FText LookAtActor_Implementation() const override;

    // Outline is enough for the workbench -- don't show the "X Interact" prompt.
    virtual bool ShouldShowInteractPrompt_Implementation() const override { return false; }

    UFUNCTION(BlueprintPure, Category = "Crafting")
    bool IsCraftingOpen() const { return CraftingViewWidget != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void OpenCrafting(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CloseCrafting();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Turns the Custom Depth outline on/off as the player enters/leaves the sphere (the post-process
    // M_outline material draws the edge). Same effect as the stone, but driven from code.
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                              bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void HandleCraftSuccess();

    
    UFUNCTION()
    void HandlePuzzleCraftRequested(FName ResultItemID, TSubclassOf<UUserWidget> PuzzleWidgetClass);

    // Turns the Custom Depth outline on/off, but stays off while crafting is open (suppressed).
    void SetOutlineEnabled(bool bEnabled);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> SphereCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FText StationName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
    TSubclassOf<UCraftingViewWidget> CraftingViewClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CloseDelayAfterCraft = 1.5f;

    // Show the interactable outline (Custom Depth) while the player is within the sphere.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Outline")
    bool bShowOutlineWhenNear = true;

    // Stencil value written to Custom Depth -- must match what the M_outline post-process expects.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Outline")
    int32 OutlineStencilValue = 1;

protected:
    UPROPERTY(Transient)
    TObjectPtr<UCraftingViewWidget> CraftingViewWidget;

    UPROPERTY(Transient)
    TObjectPtr<UInventoryComponent> ActiveInventory;

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> ActiveInteractor;

    FTimerHandle CloseAfterCraftTimer;

    // While true the outline is forced off regardless of overlap (set while crafting is open).
    bool bOutlineSuppressed = false;
};
