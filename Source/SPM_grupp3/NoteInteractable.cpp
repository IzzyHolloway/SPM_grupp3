// Fill out your copyright notice in the Description page of Project Settings.


#include "NoteInteractable.h"

//Zoey start
#include "Kismet/GameplayStatics.h"
//Zoey end
#include "OutlineComponent.h"

ANoteInteractable::ANoteInteractable()
{
	
}

void ANoteInteractable::Interact()
{
	//Zoey start
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}
	//Zoey end
	
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

	// Note is now open -- stop the interactable outline from glowing while you read it.
	if (UOutlineComponent* Outline = FindComponentByClass<UOutlineComponent>())
	{
		Outline->SetOutlineSuppressed(true);
	}

	//LockPlayerControls(NoteWidget);
    
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

	// Note closed -- let the outline light up again if the player is still nearby.
	if (UOutlineComponent* Outline = FindComponentByClass<UOutlineComponent>())
	{
		Outline->SetOutlineSuppressed(false);
	}
}
