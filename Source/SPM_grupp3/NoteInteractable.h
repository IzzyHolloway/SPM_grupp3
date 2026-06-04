// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "NoteWidgetBase.h"
#include "NoteInteractable.generated.h"

class UOutlineComponent;


/**
 * 
 */
UCLASS()
class SPM_GRUPP3_API ANoteInteractable : public AInteractableActor
{
	GENERATED_BODY()
	
public:

	ANoteInteractable();

	virtual void Interact() override;

	// The note glows with the proximity outline instead of showing the "X - Interact" prompt,
	// so we deliberately do nothing here to suppress that on-screen text.
	virtual void SetPromptVisible(bool bVisible) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note")
	FString NoteTitle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note", meta = (MultiLine = "true"))
	FString NoteText;
	UPROPERTY(EditDefaultsOnly, Category = "Note")
	TSubclassOf<UNoteWidgetBase> NoteWidgetClass;

	//Zoey start
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note")
	USoundBase* PickupSound;
	//Zoey end

protected:
	// Proximity outline -- lights up while the player is within range.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UOutlineComponent> OutlineComponent;

private:
	void LockPlayerControls(UNoteWidgetBase* NoteWidget);
	UFUNCTION()
	void OnWidgetClosed();
	UPROPERTY()
	UNoteWidgetBase* NoteWidget;
	
};
