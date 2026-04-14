// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProgressionManager.generated.h"

UCLASS()
class SPM_GRUPP3_API AProgressionManager : public AActor
{
	GENERATED_BODY()

public:
	AProgressionManager();

	UFUNCTION(BlueprintCallable)
	void AddFlag(FName FlagName);

	UFUNCTION(BlueprintCallable)
	void RemoveFlag(FName FlagName);

	UFUNCTION(BlueprintCallable)
	bool HasFlag(FName FlagName) const;

	UFUNCTION(BlueprintCallable)
	void ClearAllFlags();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	TSet<FName> ProgressFlags;
};