#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "OutlineComponent.generated.h"

/**
 * Drop this onto ANY interactable actor (Blueprint or C++). While the player is inside its radius it
 * turns on the Custom Depth outline on all of the owner's mesh components; the M_outline post-process
 * material then draws the edge. Turns it off when the player leaves. No event-graph wiring needed --
 * just add the component and set the radius.
 *
 * It is a sphere, so set its radius in the component's Details (Shape -> Sphere Radius).
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent, DisplayName="Outline"))
class SPM_GRUPP3_API UOutlineComponent : public USphereComponent
{
    GENERATED_BODY()

public:
    UOutlineComponent();

    // Stencil value written to Custom Depth -- must match what the M_outline post-process expects.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline")
    int32 OutlineStencilValue = 1;

    // Turn the outline on/off manually if you ever need to (normally driven by overlap).
    UFUNCTION(BlueprintCallable, Category = "Outline")
    void SetOutline(bool bEnabled);

    // While suppressed the outline stays OFF and overlaps are ignored. Use this when the actor is
    // "in use" so it doesn't glow the whole time -- e.g. call SetOutlineSuppressed(true) when the
    // player boards the boat and SetOutlineSuppressed(false) when they leave.
    UFUNCTION(BlueprintCallable, Category = "Outline")
    void SetOutlineSuppressed(bool bSuppressed);

protected:
    virtual void BeginPlay() override;

    // True while the player is inside the sphere -- lets us restore the correct state after unsuppressing.
    bool bPlayerInside = false;

    // While true the outline is forced off regardless of overlap.
    bool bOutlineSuppressed = false;

    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                            bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
