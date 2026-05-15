// Fill out your copyright notice in the Description page of Project Settings.

#include "DialogueWidgetBase.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UDialogueWidgetBase::SetDialogueData(const FText& SpeakerName, const FText& NewText)
{
	const FName SpeakerKey = FName(*SpeakerName.ToString());

	const FDialogueSpeakerStyle* FoundStyle = SpeakerStyles.Find(SpeakerKey);

	if (SpeakerNameText)
	{
		SpeakerNameText->SetText(SpeakerName);

		if (FoundStyle)
		{
			SpeakerNameText->SetColorAndOpacity(FSlateColor(FoundStyle->SpeakerColor));
		}
		else
		{
			SpeakerNameText->SetColorAndOpacity(FSlateColor(DefaultSpeakerColor));
		}
	}

	if (DialogueText)
	{
		DialogueText->SetText(NewText);

		// Optional: make the dialogue text same color as the speaker.
		// If you only want the speaker NAME colored, delete this color part.
		if (FoundStyle)
		{
			DialogueText->SetColorAndOpacity(FSlateColor(FoundStyle->SpeakerColor));
		}
		else
		{
			DialogueText->SetColorAndOpacity(FSlateColor(DefaultSpeakerColor));
		}
	}

	if (SpeakerPortraitImage)
	{
		if (FoundStyle && FoundStyle->SpeakerPortrait)
		{
			SpeakerPortraitImage->SetBrushFromTexture(FoundStyle->SpeakerPortrait);
			SpeakerPortraitImage->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			SpeakerPortraitImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}