// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "NoteWidgetBase.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNoteClosed);

UCLASS()
class SPM_GRUPP3_API UNoteWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable)
	void SetNoteContent(const FString& Title, const FString& Text);
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TitleText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* BodyText;
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;
	UPROPERTY(BlueprintAssignable)
	FOnNoteClosed OnNoteClosed;
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnCloseClicked();
};
