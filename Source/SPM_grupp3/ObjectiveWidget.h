// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "ObjectiveWidget.generated.h"

/**
 * Widget base class for showing the progress of the player within the current objective.
 * It includes a text block "ObjectiveProgress" which can be set through "UpdateProgressText(const FText& ProgressText)"
 */
UCLASS()
class SPM_GRUPP3_API UObjectiveWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	void UpdateProgressText(const FText& ProgressText);
	
protected:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ObjectiveProgress;
};
