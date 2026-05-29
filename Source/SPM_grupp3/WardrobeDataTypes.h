// Shared data types for the wardrobe system.

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WardrobeDataTypes.generated.h"

class UMaterialInterface;

// One row per coat in the wardrobe DataTable.
// Row name is the CoatID used everywhere else (e.g. "CoatBrown").
USTRUCT(BlueprintType)
struct FCoatDetail : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coat Data")
    FText CoatName;                  // Display name shown in the wardrobe UI.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coat Data")
    UTexture2D* CoatIcon = nullptr;  // Icon shown in the wardrobe grid (e.g. "CoatBrown_150x150").

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coat Data")
    TObjectPtr<UMaterialInterface> CoatMaterial = nullptr; // Material applied to Lumi when this coat is equipped.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coat Data")
    int32 CoatMaterialSlot = 0;      // Which material slot on Lumi's mesh to swap (usually 0).

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coat Data")
    bool bUnlockedByDefault = false; // True for the coat Lumi wears from start (e.g. CoatBrown).
};

// Runtime entry the WardrobeComponent stores. One per coat ever registered.
USTRUCT(BlueprintType)
struct FWardrobeSlot
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe")
    FName CoatID;            // Matches a row name in the coat DataTable.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe")
    bool bUnlocked = false;  // False while the matching pickup is still in the world.

    FWardrobeSlot()
    {
        CoatID = NAME_None;
        bUnlocked = false;
    }
};
