#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FCombineRecipe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FName ItemA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FName ItemB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FName ResultItem;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPM_GRUPP3_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	TArray<FCombineRecipe> AllRecipes;

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	FName CombineItems(FName InputA, FName InputB);
};