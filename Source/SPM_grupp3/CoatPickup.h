#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "Sound/SoundBase.h"
#include "CoatPickup.generated.h"

class UOutlineComponent;

/**
 * Coat pickup found in the world. Unlocks a coat in the player's UWardrobeComponent.
 * Does not touch UInventoryComponent and is not tied to story progression.
 */
UCLASS()
class SPM_GRUPP3_API ACoatPickup : public AInteractableActor
{
    GENERATED_BODY()

public:
    ACoatPickup();

    virtual void Interact() override;

protected:
    // Proximity outline -- lights up while the player is within range.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UOutlineComponent> OutlineComponent;

    // Coat row name in the wardrobe DataTable (must match exactly, e.g. "CoatRed").
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Wardrobe")
    FName CoatID = NAME_None;

    // If true, the pickup is consumed only if UnlockCoat succeeds (e.g. not already unlocked).
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Wardrobe")
    bool bRequireUnlockSuccess = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Audio")
    USoundBase* PickupSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Audio")
    float PickupSoundVolume = 0.1f;

    bool TryUnlockInWardrobe() const;
};
