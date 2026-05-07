#include "GramophoneInteractable.h"
#include "ProgressionManager.h"
#include "DialogueManager.h"
#include "Kismet/GameplayStatics.h"

AGramophoneInteractable::AGramophoneInteractable()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGramophoneInteractable::Interact()
{
	Super::Interact();

	AProgressionManager* ProgressionManager = Cast<AProgressionManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AProgressionManager::StaticClass())
	);

	ADialogueManager* DialogueManager = Cast<ADialogueManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADialogueManager::StaticClass())
	);

	if (!ProgressionManager)
	{
		return;
	}

	// Already solved / already playing.
	if (ProgressionManager->HasFlag(GramophonePlayedFlag))
	{
		if (DialogueManager)
		{
			DialogueManager->ShowMessage(FText::FromString("The gramophone is playing softly."));
		}
		return;
	}

	// Step 1: Install mechanism.
	if (!ProgressionManager->HasFlag(MechanismInstalledFlag))
	{
		if (!ProgressionManager->HasFlag(RequiredMechanismFlag))
		{
			if (DialogueManager)
			{
				DialogueManager->ShowMessage(FText::FromString("It needs some kind of mechanism."));
			}
			return;
		}

		ProgressionManager->AddFlag(MechanismInstalledFlag);

		if (DialogueManager)
		{
			DialogueManager->ShowMessage(FText::FromString("I installed the mechanism."));
		}

		return;
	}

	// Step 2: Attach shell from Island 1.
	if (!ProgressionManager->HasFlag(ShellAttachedFlag))
	{
		if (!ProgressionManager->HasFlag(RequiredShellFlag))
		{
			if (DialogueManager)
			{
				DialogueManager->ShowMessage(FText::FromString("The sound needs something to carry it."));
			}
			return;
		}

		ProgressionManager->AddFlag(ShellAttachedFlag);
		ProgressionManager->AddFlag(GramophoneReadyFlag);

		if (DialogueManager)
		{
			DialogueManager->ShowMessage(FText::FromString("The shell fits perfectly."));
		}

		return;
	}

	// Step 3: Play gramophone.
	if (ProgressionManager->HasFlag(GramophoneReadyFlag))
	{
		ProgressionManager->AddFlag(GramophonePlayedFlag);

		// Optional: keep this here if StoryFlowManager does not add it.
		ProgressionManager->AddFlag(Island2NPCExitedHouseFlag);

		if (DialogueManager)
		{
			DialogueManager->ShowMessage(FText::FromString("The melody begins to play."));
		}

		return;
	}
}