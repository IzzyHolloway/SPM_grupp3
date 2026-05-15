
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "DialogueManager.generated.h"


class UDialogueWidgetBase;

/*
 * Runtime Controller for dialogue
 * 
 * Responsibilities:
 * -Create and own dialogue widget instance
 * -Show short gameplay messages
 * -Advance and end dialogue
 * - Lock and unlock player movement during dialogue
 * 
 * 
 */


UCLASS()
class SPM_GRUPP3_API ADialogueManager : public AActor
{
	GENERATED_BODY()

public:
	ADialogueManager();

	// Show short standalone gameplay message
	UFUNCTION(BlueprintCallable)
	void ShowMessage(const FText& Message);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	float MessageDisplayTime = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue|Skip")
	float DialogueSkipHoldTime = 3.0f;

	FTimerHandle MessageHideTimerHandle;

	// Hides the dialogue/message widget
	UFUNCTION(BlueprintCallable)
	void HideMessage();

	// Starts a dialogue sequence with miltiple lines
	UFUNCTION(BlueprintCallable)
	void StartDialogue(const TArray<FDialogueLines>& InLines);
	
	// Starts dialogue sequence and stores a flag that should be added when the dialogue ends successfully
	UFUNCTION(BlueprintCallable)
	void StartDialogueWithFlag(const TArray<FDialogueLines>& InLines, FName FlagToSetOnEnd);

	// ADvance the dialogue to the next line
	UFUNCTION(BlueprintCallable)
	void AdvanceDialogue();
	
	// Called when the dialogue advance button is pressed.
	UFUNCTION(BlueprintCallable)
	void StartDialogueAdvanceHold();

	// Called when the dialogue advance button is released.
	UFUNCTION(BlueprintCallable)
	void FinishDialogueAdvanceHold();

	// Skips the whole active dialogue.
	UFUNCTION(BlueprintCallable)
	void SkipActiveDialogue();

	// Ends current dialogue, restores movement and applied pending progression if needed
	UFUNCTION(BlueprintCallable)
	void EndDialogue();

	// Return true if a multi-line dialogue is currently active
	UFUNCTION(BlueprintCallable)
	bool IsDialogueActive() const;
	
	// If true progression flag will be added when current dialogue ends
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	bool bSetFlagOnDialogueEnd = false;

	// Progression flag add when current dialogue ends
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName PendingFlagToSetOnDialogueEnd;

protected:
	virtual void BeginPlay() override;

	// Displays the current active dialogue line in widget
	void ShowCurrentDialogueLine();
	
	// Enables or disables player movement while dialogue is active
	void SetPlayerMovementEnabled(bool bEnabled);

	
	// Widget class used to display dialogue and short messages
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TSubclassOf<UDialogueWidgetBase> DialogueWidgetClass;

	// Runtime instance of dialogie widget
	UPROPERTY()
	TObjectPtr<UDialogueWidgetBase> DialogueWidgetInstance;

	// Last message or dialogue shown.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText CurrentMessage;
	
	// Timer used for holding the dialogue button to skip.
	FTimerHandle DialogueSkipTimerHandle;

	// True after the 3 second skip has happened.
	// Prevents the button release from also advancing dialogue.
	bool bDialogueSkipTriggered = false;

	// ru only while multi-line dialogue is active
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	bool bDialogueActive = false;

	// Current list of dialogue lines being played
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueLines> ActiveDialogueLines;

	// Current index inside ActiveDialogueLines
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	int32 CurrentDialogueIndex = 0;
};