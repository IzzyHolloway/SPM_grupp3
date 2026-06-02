
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "DialogueManager.generated.h"


class UDialogueWidgetBase;
class UDialogueDataAsset;

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);

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

	// Starts the first matching dialogue entry from a data asset without needing a physical NPC actor.
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogueFromDataAsset(UDialogueDataAsset* DialogueAsset);

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
	
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueEnded OnDialogueEnded;

	// Return true if a multi-line dialogue is currently active
	UFUNCTION(BlueprintCallable)
	bool IsDialogueActive() const;

	// Returns true when the current dialogue is long enough that hold-to-skip is meaningfully different from tapping once.
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Skip")
	bool CanSkipCurrentDialogue() const;
	
	// If true progression flag will be added when current dialogue ends
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	bool bSetFlagOnDialogueEnd = false;

	// Progression flag add when current dialogue ends
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName PendingFlagToSetOnDialogueEnd;
	
	UFUNCTION(BlueprintCallable)
	bool IsDialogueOrMessageVisible() const;

protected:
	virtual void BeginPlay() override;

	// Displays the current active dialogue line in widget
	void ShowCurrentDialogueLine();

	// Returns true if the dialogue entry matches the current progression state.
	bool DoesDialogueEntryMatch(class AProgressionManager* ProgressionManager, const struct FDialogueEntry& Entry) const;
	
	// Enables or disables player movement while dialogue is active
	void SetPlayerMovementEnabled(bool bEnabled);

	// Hides and resets the skip prompt so it does not linger between messages or dialogue lines.
	void ResetDialogueSkipPrompt();

	// Updates the passive conversation hint that should remain visible during multi-line dialogue.
	void UpdateConversationSkipHint();

	
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

	// True while the player is currently holding the dialogue advance button and the skip prompt is active.
	bool bDialogueSkipHoldActive = false;

	// ru only while multi-line dialogue is active
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	bool bDialogueActive = false;

	// Current list of dialogue lines being played
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueLines> ActiveDialogueLines;

	// Current index inside ActiveDialogueLines
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	int32 CurrentDialogueIndex = 0;
	
private:
	bool bMessageVisible = false;
};
