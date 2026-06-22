#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CutsceneWidgetBase.generated.h"

/**
 * Shared base for every cutscene widget (the WBP_Cutscene* family that shows the video cutscenes).
 *
 * While a cutscene widget is on screen the player is frozen: movement is disabled and move/look
 * input is ignored. The lock happens in NativeConstruct and the unlock in NativeDestruct, so
 * control is ALWAYS restored when the widget is removed -- whether the cutscene ends on its own or
 * is skipped -- and there is no "forgot to unlock" path (the bug pattern we hit with the door).
 *
 * To use: reparent each cutscene WBP to this class (Class Settings -> Parent Class =
 * CutsceneWidgetBase). No Blueprint wiring needed.
 */
UCLASS()
class SPM_GRUPP3_API UCutsceneWidgetBase : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // Uncheck on a specific cutscene widget if that one should still let the player move.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
    bool bLockPlayerDuringCutscene = true;

private:
    void SetPlayerLocked(bool bLocked);
};
