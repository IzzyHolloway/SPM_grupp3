// Fill out your copyright notice in the Description page of Project Settings.


#include "NoteWidgetBase.h"


void UNoteWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UNoteWidgetBase::OnCloseClicked);
	}
}

void UNoteWidgetBase::SetNoteContent(const FString& Title, const FString& Text)
{
	UE_LOG(LogTemp, Warning, TEXT("We made it to the SetNoteContent"));
	
	if (TitleText && BodyText)
	{
		TitleText->SetText(FText::FromString(Title));
		BodyText->SetText(FText::FromString(Text));
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("The Text could not be set"));
	}
	
}

void UNoteWidgetBase::OnCloseClicked()
{
	OnNoteClosed.Broadcast();
	RemoveFromViewport();
}


