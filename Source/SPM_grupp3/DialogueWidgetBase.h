// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sound/SoundBase.h"
#include "DialogueWidgetBase.generated.h"


//Zoeys: #include "Sound/SoundBase.h"

class UTextBlock;
class UImage;
class UTexture2D;

//Zoey Start
class UAudioComponent;  
//Zoey end

USTRUCT(BlueprintType)
struct FDialogueSpeakerStyle
{	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FLinearColor SpeakerColor = FLinearColor::White;
	
	//
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	//FLinearColor SpeakerDialogueColor = FLinearColor::White;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TObjectPtr<UTexture2D> SpeakerPortrait = nullptr;
	
	//Zoey start
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TObjectPtr<USoundBase> SpeakerSound = nullptr;
	//Zoey end
};

UCLASS()
class SPM_GRUPP3_API UDialogueWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetDialogueData(const FText& SpeakerName, const FText& NewText);

	//Zoey start
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<USoundBase> DefaultDialogueSound = nullptr;
    
	UPROPERTY()
	TObjectPtr<UAudioComponent> DialogueAudioComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TSet<FName> SpeakersWithoutSound;
	//Zoey end
	
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