// Fill out your copyright notice in the Description page of Project Settings.

#include "DialogueWidgetBase.h"
#include "Components/TextBlock.h"

void UDialogueWidgetBase::SetDialogueData(const FText& SpeakerName, const FText& NewText)
{
	if (SpeakerNameText)
	{
		SpeakerNameText->SetText(SpeakerName);
	}

	if (DialogueText)
	{
		DialogueText->SetText(NewText);
	}
}