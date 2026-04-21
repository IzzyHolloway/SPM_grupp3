// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ObjectiveWidgetBase.generated.h"

class UTextBlock;

/**
 * Very simple debug/prototype objective widget.
 *
 * Responsibilities:
 * -Display current objective text
 * - Optionally display current objective ID for debugging
 *
 * displays data provided by ProgressionManager.
 */
UCLASS()
class SPM_GRUPP3_API UObjectiveWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	// Updates the widget with the latest objective text and ID.
	UFUNCTION(BlueprintCallable)
	void SetObjectiveData(const FText& ObjectiveText, const FText& ObjectiveIDText);

protected:
	//Main visible objective text.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ObjectiveTextBlock;

	//Optional debug text for the objective ID.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ObjectiveIDTextBlock;
};