#include "DialogueManager.h"
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
		}
	}
}

void ADialogueManager::ShowMessage(const FText& Message)
{
	CurrentMessage = Message;

	// Short gameplay messages are not considered "active dialogue".
	bDialogueActive = false;

	if (DialogueWidgetInstance)
	{
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
	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(MessageHideTimerHandle);
	}
}

void ADialogueManager::StartDialogue(const TArray<FDialogueLines>& InLines)
{
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

void ADialogueManager::EndDialogue()
{
	bDialogueActive = false;
	CurrentDialogueIndex = 0;
	ActiveDialogueLines.Empty();

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
}

bool ADialogueManager::IsDialogueActive() const
{
	return bDialogueActive;
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

	SetPlayerMovementEnabled(false);
	ShowCurrentDialogueLine();
}