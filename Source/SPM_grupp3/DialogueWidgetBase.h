// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueWidgetBase.generated.h"

class UTextBlock;
class UImage;
class UTexture2D;

USTRUCT(BlueprintType)
struct FDialogueSpeakerStyle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FLinearColor SpeakerColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TObjectPtr<UTexture2D> SpeakerPortrait = nullptr;
};

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

	// Optional image in the widget. Name the Image widget "SpeakerPortraitImage" in WBP.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> SpeakerPortraitImage;

	// Set these in the Widget Blueprint defaults.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TMap<FName, FDialogueSpeakerStyle> SpeakerStyles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FLinearColor DefaultSpeakerColor = FLinearColor::White;
};