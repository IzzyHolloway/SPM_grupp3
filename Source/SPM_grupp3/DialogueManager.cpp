#include "DialogueManager.h"
#include "DialogueWidgetBase.h"
#include "CharacterAimi.h"

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

	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		DialogueWidgetInstance->SetDialogueText(Message);
	}
}

void ADialogueManager::HideMessage()
{
	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ADialogueManager::StartDialogue(const TArray<FText>& InLines)
{
	if (InLines.IsEmpty())
	{
		return;
	}

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

	ShowMessage(ActiveDialogueLines[CurrentDialogueIndex]);
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