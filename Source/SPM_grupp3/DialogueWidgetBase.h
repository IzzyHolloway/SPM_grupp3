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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueSkipPromptCompleted);

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

	// Called by the nested skip prompt widget when the hold-to-skip animation reaches 100%.
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Skip")
	void NotifySkipPromptCompleted();

	// Starts the dialogue skip prompt and lets Blueprint drive the visual hold animation.
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue|Skip")
	void StartSkipPromptHold(float HoldDuration);

	// Cancels the current skip hold and hides or rewinds the prompt.
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue|Skip")
	void CancelSkipPromptHold();

	// Lets Blueprint play a short "skip confirmed" animation before the dialogue closes.
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue|Skip")
	void CompleteSkipPromptHold();

	// Resets the skip prompt to its hidden idle state.
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue|Skip")
	void ResetSkipPrompt();

	// Shows or hides the passive "hold to skip" hint used during longer conversations.
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue|Skip")
	void SetConversationSkipHintVisible(bool bVisible);

	// Dialogue manager listens to this instead of needing direct access to the nested skip prompt widget.
	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Skip")
	FOnDialogueSkipPromptCompleted OnSkipPromptCompleted;

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
