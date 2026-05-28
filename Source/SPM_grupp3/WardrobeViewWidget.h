#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WardrobeViewWidget.generated.h"

class UWardrobeComponent;
class AWardrobe;

UCLASS()
class SPM_GRUPP3_API UWardrobeViewWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Wardrobe")
    TObjectPtr<UWardrobeComponent> Wardrobe;

    UPROPERTY(BlueprintReadWrite, Category = "Wardrobe")
    TObjectPtr<AWardrobe> WardrobeActor;
};
