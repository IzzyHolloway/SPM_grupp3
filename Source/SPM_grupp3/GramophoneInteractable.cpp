#include "GramophoneInteractable.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"
#include "DialogueManager.h"

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

	if (ProgressionManager->HasFlag(GramophonePlayedFlag))
	{
		if (DialogueManager)
		{
			DialogueManager->ShowMessage(FText::FromString("The gramophone is playing softly."));
		}
		return;
	}

	if (!ProgressionManager->HasFlag(MechanismInstalledFlag))
	{
		if (!ProgressionManager->HasFlag(RequiredMechanismFlag))
		{
			if (DialogueManager)
			{
				DialogueManager->ShowMessage(FText::FromString("It is missing its mechanism."));
			}
			return;
		}

		ProgressionManager->AddFlag(MechanismInstalledFlag);

		if (DialogueManager)
		{
			DialogueManager->ShowMessage(FText::FromString("I installed the gramophone mechanism."));
		}
		return;
	}

	ProgressionManager->AddFlag(GramophonePlayedFlag);
	ProgressionManager->AddFlag(Island2NPCExitedHouseFlag);

	if (DialogueManager)
	{
		DialogueManager->ShowMessage(FText::FromString("The melody begins to play."));
	}
}