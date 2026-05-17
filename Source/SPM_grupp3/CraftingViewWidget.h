#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CraftingViewWidget.generated.h"

class UInventoryComponent;
class ACraftingStation;

UCLASS()
class SPM_GRUPP3_API UCraftingViewWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Crafting")
    TObjectPtr<UInventoryComponent> CraftingInventory;

    UPROPERTY(BlueprintReadWrite, Category = "Crafting")
    TObjectPtr<ACraftingStation> CraftingStation;
};
