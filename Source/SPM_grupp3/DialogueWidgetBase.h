// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueWidgetBase.generated.h"

class UTextBlock;

UCLASS()
class SPM_GRUPP3_API UDialogueWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetDialogueData(const FText& SpeakerName, const FText& NewText);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeakerNameText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DialogueText;
};