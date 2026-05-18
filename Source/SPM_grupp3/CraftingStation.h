#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "CraftingStation.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UCraftingViewWidget;
class UInventoryComponent;

UCLASS()
class SPM_GRUPP3_API ACraftingStation : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    ACraftingStation();

    // IInteractable
    virtual void InteractWith_Implementation(AActor* Interactor) override;
    virtual FText LookAtActor_Implementation() const override;

    UFUNCTION(BlueprintPure, Category = "Crafting")
    bool IsCraftingOpen() const { return CraftingViewWidget != nullptr; }

    /** Opens the crafting view bound to the given interactor's inventory. */
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void OpenCrafting(AActor* Interactor);

    /** Closes the crafting view if it is open. Safe to call when closed. */
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CloseCrafting();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void HandleCraftSuccess();

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> SphereCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FText StationName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
    TSubclassOf<UCraftingViewWidget> CraftingViewClass;

    /** Seconds to wait after a successful craft before closing the station. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CloseDelayAfterCraft = 1.5f;

protected:
    UPROPERTY(Transient)
    TObjectPtr<UCraftingViewWidget> CraftingViewWidget;

    UPROPERTY(Transient)
    TObjectPtr<UInventoryComponent> ActiveInventory;

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> ActiveInteractor;

    FTimerHandle CloseAfterCraftTimer;
};
