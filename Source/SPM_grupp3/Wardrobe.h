#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "Wardrobe.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UWardrobeViewWidget;
class UWardrobeComponent;

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

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> SphereCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe")
    FText StationName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wardrobe")
    TSubclassOf<UWardrobeViewWidget> WardrobeViewClass;

protected:
    UPROPERTY(Transient)
    TObjectPtr<UWardrobeViewWidget> WardrobeViewWidget;

    UPROPERTY(Transient)
    TObjectPtr<UWardrobeComponent> ActiveWardrobe;

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> ActiveInteractor;
};
