// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveWidget.h"

void UObjectiveWidget::UpdateProgressText(const FText& ProgressText)
{
	if (ObjectiveProgress)
	{
		ObjectiveProgress->SetText(ProgressText);
	}
}