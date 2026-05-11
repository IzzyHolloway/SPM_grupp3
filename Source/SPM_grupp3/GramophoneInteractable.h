#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "GramophoneInteractable.generated.h"

class AProgressionManager;

//Zoey Start
class UAudioComponent;
class USoundBase;
class USoundAttenuation;
//Zoey End

UCLASS()
class SPM_GRUPP3_API AGramophoneInteractable : public AInteractableActor
{
	GENERATED_BODY()

public:
	AGramophoneInteractable();

	virtual void Interact() override;

protected:
	
	//Zoey Start
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	UAudioComponent* AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* GramophoneSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundAttenuation* AttenuationSettings;
	//Zoey End
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName RequiredMechanismFlag = "GramophoneMechanismCrafted";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName MechanismInstalledFlag = "GramophoneMechanismInstalled";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName GramophonePlayedFlag = "GramophonePlayed";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FName Island2NPCExitedHouseFlag = "Island2NPCExitedHouse";
};