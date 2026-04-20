// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "NoteWidgetBase.h"
#include "NoteInteractable.generated.h"


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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note")
	FString NoteTitle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note", meta = (MultiLine = "true"))
	FString NoteText;
	UPROPERTY(EditDefaultsOnly, Category = "Note")
	TSubclassOf<UNoteWidgetBase> NoteWidgetClass;

	
private:
	void LockPlayerControls(UNoteWidgetBase* NoteWidget);
	UFUNCTION()
	void OnWidgetClosed();
	UPROPERTY()
	UNoteWidgetBase* NoteWidget;
};
