// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "ObjectiveWidget.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GRUPP3_API UObjectiveWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	// void UpdateObjectiveDescriptionText(const FText& DescriptionText);
	void UpdateProgressText(const FText& ProgressText);
	// void UpdateImage(UTexture2D* Image);
	
protected:
	
	/*
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ObjectiveDescription;
	*/

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ObjectiveProgress;

	/*
	UPROPERTY(meta = (BindWidget))
	UImage* ObjectiveImage;
	*/
	
};
