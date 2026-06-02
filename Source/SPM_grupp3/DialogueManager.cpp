#include "DialogueManager.h"
#include "DialogueDataAsset.h"
#include "DialogueEntry.h"
#include "DialogueWidgetBase.h"
#include "CharacterAimi.h"
#include "DialogueLines.h"
#include "ProgressionManager.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ADialogueManager::ADialogueManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADialogueManager::BeginPlay()
{
	Super::BeginPlay();

	if (DialogueWidgetClass)
	{
		DialogueWidgetInstance = CreateWidget<UDialogueWidgetBase>(GetWorld(), DialogueWidgetClass);

		if (DialogueWidgetInstance)
		{
			DialogueWidgetInstance->AddToViewport();
			DialogueWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
			DialogueWidgetInstance->OnSkipPromptCompleted.AddDynamic(this, &ADialogueManager::SkipActiveDialogue);
			DialogueWidgetInstance->ResetSkipPrompt();
			DialogueWidgetInstance->SetConversationSkipHintVisible(false);
		}
	}
}

void ADialogueManager::ShowMessage(const FText& Message)
{
	CurrentMessage = Message;

	// Short gameplay messages are not considered "active dialogue".
	bDialogueActive = false;
	bMessageVisible = true;
	bDialogueSkipHoldActive = false;

	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->ResetSkipPrompt();
		DialogueWidgetInstance->SetConversationSkipHintVisible(false);
		DialogueWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		DialogueWidgetInstance->SetDialogueData(FText::GetEmpty(), Message);
	}

	// Reset and restart the auto-hide timer for short messages.
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(MessageHideTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			MessageHideTimerHandle,
			this,
			&ADialogueManager::HideMessage,
			MessageDisplayTime,
			false
		);
	}
}

void ADialogueManager::HideMessage()
{
	bMessageVisible = false;
	bDialogueSkipHoldActive = false;
	
	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->ResetSkipPrompt();
		DialogueWidgetInstance->SetConversationSkipHintVisible(false);
		DialogueWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(MessageHideTimerHandle);
	}
}

void ADialogueManager::StartDialogue(const TArray<FDialogueLines>& InLines)
{
	
	GetWorldTimerManager().ClearTimer(DialogueSkipTimerHandle);
	bDialogueSkipTriggered = false;
	bDialogueSkipHoldActive = false;
	
	if (InLines.IsEmpty())
	{
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(MessageHideTimerHandle);
	}

	// No pending progression update for this dialogue.
	bSetFlagOnDialogueEnd = false;
	PendingFlagToSetOnDialogueEnd = NAME_None;

	ActiveDialogueLines = InLines;
	CurrentDialogueIndex = 0;
	bDialogueActive = true;

	ResetDialogueSkipPrompt();
	UpdateConversationSkipHint();
	SetPlayerMovementEnabled(false);
	ShowCurrentDialogueLine();
}

void ADialogueManager::AdvanceDialogue()
{
	if (!bDialogueActive)
	{
		return;
	}

	CurrentDialogueIndex++;

	// If there are no more lines, finish the dialogue.
	if (!ActiveDialogueLines.IsValidIndex(CurrentDialogueIndex))
	{
		EndDialogue();
		return;
	}

	ShowCurrentDialogueLine();
}

void ADialogueManager::StartDialogueAdvanceHold()
{
	if (!bDialogueActive)
	{
		return;
	}

	bDialogueSkipTriggered = false;
	bDialogueSkipHoldActive = false;

	GetWorldTimerManager().ClearTimer(DialogueSkipTimerHandle);

	if (!CanSkipCurrentDialogue())
	{
		ResetDialogueSkipPrompt();
		return;
	}

	bDialogueSkipHoldActive = true;

	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->SetConversationSkipHintVisible(false);
		DialogueWidgetInstance->StartSkipPromptHold(DialogueSkipHoldTime);
	}
}

void ADialogueManager::FinishDialogueAdvanceHold()
{
	if (!bDialogueActive)
	{
		GetWorldTimerManager().ClearTimer(DialogueSkipTimerHandle);
		if (DialogueWidgetInstance)
		{
			DialogueWidgetInstance->CancelSkipPromptHold();
		}
		ResetDialogueSkipPrompt();
		return;
	}

	GetWorldTimerManager().ClearTimer(DialogueSkipTimerHandle);
	bDialogueSkipHoldActive = false;

	if (bDialogueSkipTriggered)
	{
		bDialogueSkipTriggered = false;
		ResetDialogueSkipPrompt();
		UpdateConversationSkipHint();
		return;
	}

	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->CancelSkipPromptHold();
	}

	ResetDialogueSkipPrompt();
	AdvanceDialogue();
}

void ADialogueManager::SkipActiveDialogue()
{
	if (!bDialogueActive)
	{
		return;
	}

	bDialogueSkipTriggered = true;
	bDialogueSkipHoldActive = false;

	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->CompleteSkipPromptHold();
	}

	EndDialogue();
}

void ADialogueManager::EndDialogue()
{
	
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DialogueSkipTimerHandle);
	}

	bDialogueSkipTriggered = false;
	bDialogueSkipHoldActive = false;
	
	bDialogueActive = false;
	CurrentDialogueIndex = 0;
	ActiveDialogueLines.Empty();

	ResetDialogueSkipPrompt();
	UpdateConversationSkipHint();
	HideMessage();
	SetPlayerMovementEnabled(true);

	// Apply pending progression update only after the dialogue fully completes.
	if (bSetFlagOnDialogueEnd && !PendingFlagToSetOnDialogueEnd.IsNone())
	{
		AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
		);

		if (ProgressionManager)
		{
			ProgressionManager->AddFlag(PendingFlagToSetOnDialogueEnd);
		}
	}

	// Clear pending progression state after use.
	bSetFlagOnDialogueEnd = false;
	PendingFlagToSetOnDialogueEnd = NAME_None;
	
	OnDialogueEnded.Broadcast();
}

bool ADialogueManager::IsDialogueActive() const
{
	return bDialogueActive;
}

bool ADialogueManager::CanSkipCurrentDialogue() const
{
	if (!bDialogueActive)
	{
		return false;
	}

	// A hold-to-skip prompt is most useful when there is more than one line left to skip.
	return ActiveDialogueLines.Num() - CurrentDialogueIndex > 1;
}

void ADialogueManager::ShowCurrentDialogueLine()
{
	if (!ActiveDialogueLines.IsValidIndex(CurrentDialogueIndex))
	{
		return;
	}

	const FDialogueLines& CurrentLine = ActiveDialogueLines[CurrentDialogueIndex];

	CurrentMessage = CurrentLine.LineText;

	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->ResetSkipPrompt();
		UpdateConversationSkipHint();
		DialogueWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		DialogueWidgetInstance->SetDialogueData(CurrentLine.SpeakerName, CurrentLine.LineText);
	}
}

void ADialogueManager::SetPlayerMovementEnabled(bool bEnabled)
{
	ACharacterAimi* PlayerCharacter = Cast<ACharacterAimi>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
	);

	if (!PlayerCharacter)
	{
		return;
	}

	if (UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement())
	{
		if (bEnabled)
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}
		else
		{
			MoveComp->DisableMovement();
		}
	}
}

void ADialogueManager::StartDialogueWithFlag(const TArray<FDialogueLines>& InLines, FName FlagToSetOnEnd)
{
	GetWorldTimerManager().ClearTimer(DialogueSkipTimerHandle);
	bDialogueSkipTriggered = false;
	bDialogueSkipHoldActive = false;
	
	if (InLines.IsEmpty())
	{
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(MessageHideTimerHandle);
	}

	ActiveDialogueLines = InLines;
	CurrentDialogueIndex = 0;
	bDialogueActive = true;

	// Store progression flag to be awarded after the dialogue completes
	bSetFlagOnDialogueEnd = !FlagToSetOnEnd.IsNone();
	PendingFlagToSetOnDialogueEnd = FlagToSetOnEnd;

	ResetDialogueSkipPrompt();
	UpdateConversationSkipHint();
	SetPlayerMovementEnabled(false);
	ShowCurrentDialogueLine();
}

void ADialogueManager::StartDialogueFromDataAsset(UDialogueDataAsset* DialogueAsset)
{
	if (!DialogueAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartDialogueFromDataAsset failed: DialogueAsset is null."));
		return;
	}

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	for (const FDialogueEntry& Entry : DialogueAsset->DialogueEntries)
	{
		if (!DoesDialogueEntryMatch(ProgressionManager, Entry))
		{
			continue;
		}

		if (Entry.DialogueLines.IsEmpty())
		{
			continue;
		}

		if (Entry.bSetFlagOnDialogueEnd && !Entry.FlagToSetOnDialogueEnd.IsNone())
		{
			StartDialogueWithFlag(Entry.DialogueLines, Entry.FlagToSetOnDialogueEnd);
		}
		else
		{
			StartDialogue(Entry.DialogueLines);
		}

		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("StartDialogueFromDataAsset found no matching dialogue entry in asset %s."), *DialogueAsset->GetName());
}

bool ADialogueManager::IsDialogueOrMessageVisible() const
{
	return bDialogueActive || bMessageVisible;
}

bool ADialogueManager::DoesDialogueEntryMatch(AProgressionManager* ProgressionManager, const FDialogueEntry& Entry) const
{
	// If progression checks are needed but the manager does not exist, fail safely.
	if (!ProgressionManager && (!Entry.RequiredFlags.IsEmpty() || !Entry.BlockedFlags.IsEmpty()))
	{
		return false;
	}

	// Every required flag must exist.
	for (const FName& RequiredFlag : Entry.RequiredFlags)
	{
		if (RequiredFlag.IsNone())
		{
			continue;
		}

		if (!ProgressionManager->HasFlag(RequiredFlag))
		{
			return false;
		}
	}

	// None of the blocked flags may exist.
	for (const FName& BlockedFlag : Entry.BlockedFlags)
	{
		if (BlockedFlag.IsNone())
		{
			continue;
		}

		if (ProgressionManager->HasFlag(BlockedFlag))
		{
			return false;
		}
	}

	return true;
}

void ADialogueManager::ResetDialogueSkipPrompt()
{
	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->ResetSkipPrompt();
	}
}

void ADialogueManager::UpdateConversationSkipHint()
{
	if (!DialogueWidgetInstance)
	{
		return;
	}

	// Show the passive skip hint only for real conversations, not for one-line messages.
	const bool bShowConversationHint = bDialogueActive && ActiveDialogueLines.Num() > 1 && !bDialogueSkipHoldActive;
	DialogueWidgetInstance->SetConversationSkipHintVisible(bShowConversationHint);
}
