// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "PanelInteractable.generated.h"

/**
 * 
 */
UCLASS()
class SPM_GRUPP3_API APanelInteractable : public AInteractableActor
{
	GENERATED_BODY()
	
public:
	APanelInteractable();
	
	//Event that is created in the Blueprint. 
	UFUNCTION(BlueprintImplementableEvent)
	void OnInteract(); 
	
	UFUNCTION(BlueprintImplementableEvent)
	void CloseInteraction();
	
	virtual void Interact() override;
	
	virtual void EndInteraction();
	
	UPROPERTY(BlueprintReadWrite)
	TArray<int32> PlayerInput;

	UFUNCTION(BlueprintCallable)
	void ClearInput();
	
};
