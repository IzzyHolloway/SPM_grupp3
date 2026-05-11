// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorInteractable.h"

ADoorInteractable::ADoorInteractable()
{
	
}

//Calling the Blueprint event when we interact with the door. 
void ADoorInteractable::Interact()
{
	OnDoorInteract();
}
