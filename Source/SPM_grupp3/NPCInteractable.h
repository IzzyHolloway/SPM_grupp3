// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "DialogueLines.h"
#include "NPCInteractable.generated.h"

UCLASS()
class SPM_GRUPP3_API ANPCInteractable : public AInteractableActor
{
	GENERATED_BODY()

public:
	virtual void Interact() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueLines> DialogueLines;
};