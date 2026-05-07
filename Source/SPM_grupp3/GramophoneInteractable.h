#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "GramophoneInteractable.generated.h"

class AProgressionManager;

UCLASS()
class SPM_GRUPP3_API AGramophoneInteractable : public AInteractableActor
{
	GENERATED_BODY()

public:
	AGramophoneInteractable();

	virtual void Interact() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName RequiredMechanismFlag = "GramophoneMechanismCrafted";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName MechanismInstalledFlag = "GramophoneMechanismInstalled";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName RequiredShellFlag = "ShellReceivedFromIsland1";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName ShellAttachedFlag = "ShellAttachedToGramophone";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName GramophoneReadyFlag = "GramophoneReady";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName GramophonePlayedFlag = "GramophonePlayed";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName Island2NPCExitedHouseFlag = "Island2NPCExitedHouse";
};