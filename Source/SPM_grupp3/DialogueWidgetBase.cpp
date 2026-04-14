// Fill out your copyright notice in the Description page of Project Settings.

#include "DialogueWidgetBase.h"
#include "Components/TextBlock.h"

void UDialogueWidgetBase::SetDialogueText(const FText& NewText)
{
	if (DialogueText)
	{
		DialogueText->SetText(NewText);
	}
}