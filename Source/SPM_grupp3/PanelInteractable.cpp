// Fill out your copyright notice in the Description page of Project Settings.


#include "PanelInteractable.h"

#include "NoteInteractable.h"
#include "OutlineComponent.h"

APanelInteractable::APanelInteractable()
{

}


//Calls the Blueprint event on interaction.
void APanelInteractable::Interact()
{
	// Keypad is now open -- stop the outline from glowing while the player stands at the panel.
	SetOutlineSuppressed(true);

	OnInteract();
}

void APanelInteractable::ClearInput()
{
	PlayerInput.Empty();
}

void APanelInteractable::EndInteraction()
{
	// Panel closed -- let the outline light up again if the player is still nearby.
	SetOutlineSuppressed(false);

	CloseInteraction();
}

void APanelInteractable::OnPuzzleSolved()
{
	UE_LOG(LogTemp, Warning, TEXT("APanelInteractable::OnPuzzleSolved called on %s -- destroying it now."),
		*GetName());

	// Close the keypad UI and restore the outline state, then remove the panel entirely.
	EndInteraction();
	Destroy();
}

void APanelInteractable::SetOutlineSuppressed(bool bSuppressed)
{
	if (UOutlineComponent* Outline = FindComponentByClass<UOutlineComponent>())
	{
		Outline->SetOutlineSuppressed(bSuppressed);
	}
}
