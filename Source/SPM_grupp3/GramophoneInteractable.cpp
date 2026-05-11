#include "GramophoneInteractable.h"
#include "ProgressionManager.h"
#include "Kismet/GameplayStatics.h"
#include "DialogueManager.h"

//Zoey Start
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
//Zoey End


AGramophoneInteractable::AGramophoneInteractable()
{
	PrimaryActorTick.bCanEverTick = false;
	
	//Zoey Start
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));

	AudioComponent->bAutoActivate = false;
	AudioComponent->bIsUISound = false;
	//Zoey End
}

//Zoey Start
void AGramophoneInteractable::BeginPlay()
{
	Super::BeginPlay();

	if (AudioComponent)
	{
		AudioComponent->SetSound(GramophoneSound);
		AudioComponent->AttenuationSettings = AttenuationSettings;
	}
}
//Zoey End

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
	
	//Zoey Start
	if (AudioComponent && !AudioComponent->IsPlaying())
	{
		AudioComponent->Play();
	}
	//Zoey End

	if (DialogueManager)
	{
		DialogueManager->ShowMessage(FText::FromString("The melody begins to play."));
	}
}