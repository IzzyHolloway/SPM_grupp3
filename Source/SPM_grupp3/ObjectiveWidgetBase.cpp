// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveWidgetBase.h"
#include "Components/TextBlock.h"

void UObjectiveWidgetBase::SetObjectiveData(const FText& ObjectiveText, const FText& ObjectiveIDText)
{
	if (ObjectiveTextBlock)
	{
		ObjectiveTextBlock->SetText(ObjectiveText);
	}

	if (ObjectiveIDTextBlock)
	{
		ObjectiveIDTextBlock->SetText(ObjectiveIDText);
	}
}