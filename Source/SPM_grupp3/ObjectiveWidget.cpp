// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveWidget.h"

void UObjectiveWidget::UpdateObjectiveDescriptionText(const FText& DescriptionText)
{
	if (ObjectiveDescription)
	{
		ObjectiveDescription->SetText(DescriptionText);
	}
}

void UObjectiveWidget::UpdateProgressText(const FText& ProgressText)
{
	if (ObjectiveProgress)
	{
		ObjectiveProgress->SetText(ProgressText);
	}
}

void UObjectiveWidget::UpdateImage(UTexture2D* Image)
{
	if (ObjectiveImage)
	{
		ObjectiveImage->SetBrushFromTexture(Image);
	}
}
