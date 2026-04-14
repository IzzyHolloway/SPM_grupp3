
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "DialogueManager.generated.h"


class UDialogueWidgetBase;

UCLASS()
class SPM_GRUPP3_API ADialogueManager : public AActor
{
	GENERATED_BODY()

public:
	ADialogueManager();

	UFUNCTION(BlueprintCallable)
	void ShowMessage(const FText& Message);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	float MessageDisplayTime = 2.0f;

	FTimerHandle MessageHideTimerHandle;

	UFUNCTION(BlueprintCallable)
	void HideMessage();

	UFUNCTION(BlueprintCallable)
	void StartDialogue(const TArray<FDialogueLines>& InLines);

	UFUNCTION(BlueprintCallable)
	void AdvanceDialogue();

	UFUNCTION(BlueprintCallable)
	void EndDialogue();

	UFUNCTION(BlueprintCallable)
	bool IsDialogueActive() const;

protected:
	virtual void BeginPlay() override;

	void ShowCurrentDialogueLine();
	void SetPlayerMovementEnabled(bool bEnabled);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TSubclassOf<UDialogueWidgetBase> DialogueWidgetClass;

	UPROPERTY()
	TObjectPtr<UDialogueWidgetBase> DialogueWidgetInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText CurrentMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	bool bDialogueActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueLines> ActiveDialogueLines;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	int32 CurrentDialogueIndex = 0;
};