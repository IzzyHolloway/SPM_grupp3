// Fill out your copyright notice in the Description page of Project Settings.


#include "PanelInteractable.h"

#include "NoteInteractable.h"

APanelInteractable::APanelInteractable()
{
	
}


//Calls the Blueprint event on interaction. 
void APanelInteractable::Interact()
{
	OnInteract();
}

void APanelInteractable::ClearInput()
{
	PlayerInput.Empty();
}

void APanelInteractable::EndInteraction()
{
	CloseInteraction();
}
