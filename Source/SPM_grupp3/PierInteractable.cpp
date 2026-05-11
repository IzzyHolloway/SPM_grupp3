#include "PierInteractable.h"
#include "ProgressionManager.h"
#include "DialogueManager.h"

#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

APierInteractable::APierInteractable()
{
	PlayerExitPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerExitPoint"));
	PlayerExitPoint->SetupAttachment(RootComponent);
	PlayerExitPoint->SetRelativeLocation(FVector(150.f, 0.f, 80.f));
}

void APierInteractable::Interact()
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
		UE_LOG(LogTemp, Warning, TEXT("PierInteractable: No ProgressionManager found."));
		return;
	}

	ACharacter* PlayerCharacter = GetPlayerCharacter();

	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("PierInteractable: No player character found."));
		return;
	}

	if (!CanDock(ProgressionManager))
	{
		if (DialogueManager)
		{
			DialogueManager->ShowMessage(BlockedMessage);
		}

		return;
	}

	AddSuccessFlag(ProgressionManager);

	if (DialogueManager)
	{
		DialogueManager->ShowMessage(SuccessMessage);
	}

	const FVector ExitLocation = PlayerExitPoint
		? PlayerExitPoint->GetComponentLocation()
		: GetActorLocation() + FVector(0.f, 0.f, 100.f);

	OnDockingAllowed(PlayerCharacter, ExitLocation);
}

bool APierInteractable::CanDock(AProgressionManager* ProgressionManager) const
{
	if (!ProgressionManager)
	{
		return false;
	}

	if (RequiredProgressFlag.IsNone())
	{
		return true;
	}

	return ProgressionManager->HasFlag(RequiredProgressFlag);
}

void APierInteractable::AddSuccessFlag(AProgressionManager* ProgressionManager) const
{
	if (!ProgressionManager)
	{
		return;
	}

	if (!ProgressFlagToAddOnSuccess.IsNone())
	{
		ProgressionManager->AddFlag(ProgressFlagToAddOnSuccess);
	}
}

ACharacter* APierInteractable::GetPlayerCharacter() const
{
	return Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}