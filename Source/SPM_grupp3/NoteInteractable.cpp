// Fill out your copyright notice in the Description page of Project Settings.


#include "NoteInteractable.h"



ANoteInteractable::ANoteInteractable()
{
	
}

void ANoteInteractable::Interact()
{
	if (NoteWidget && NoteWidget->IsInViewport()) 
	{
		return;
	}

	if (!NoteWidgetClass) return;
    
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	UNoteWidgetBase* Widget = CreateWidget<UNoteWidgetBase>(PC, NoteWidgetClass);
    
	if (!Widget) return;
    
	Widget->SetNoteContent(NoteTitle, NoteText);
	Widget->AddToViewport();
    
	NoteWidget = Widget;
    
	LockPlayerControls(NoteWidget);
    
	Widget->OnNoteClosed.AddDynamic(this, &ANoteInteractable::OnWidgetClosed);
}

void ANoteInteractable::LockPlayerControls(UNoteWidgetBase* Widget)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(true);
}

void ANoteInteractable::OnWidgetClosed()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	PC->SetShowMouseCursor(false);
	PC->SetInputMode(FInputModeGameOnly());
	NoteWidget = nullptr;
}
