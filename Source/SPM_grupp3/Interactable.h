#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
    GENERATED_BODY()
};

class SPM_GRUPP3_API IInteractable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void InteractWith(AActor* Interactor);
    virtual void InteractWith_Implementation(AActor* /*Interactor*/) {}

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FText LookAtActor() const;
    virtual FText LookAtActor_Implementation() const { return FText::GetEmpty(); }
};
