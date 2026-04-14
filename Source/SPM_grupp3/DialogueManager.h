
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueManager.generated.h"

class UDialogueWidgetBase;

UCLASS()
class SPM_GRUPP3_API ADialogueManager : public AActor
{
	GENERATED_BODY()

public:
	ADialogueManager();

	UFUNCTION(BlueprintCallable)
	void ShowMessage(const FText& Message);

	UFUNCTION(BlueprintCallable)
	void HideMessage();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TSubclassOf<UDialogueWidgetBase> DialogueWidgetClass;

	UPROPERTY()
	TObjectPtr<UDialogueWidgetBase> DialogueWidgetInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText CurrentMessage;
};