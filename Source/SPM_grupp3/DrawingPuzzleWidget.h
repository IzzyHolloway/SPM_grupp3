// //Izzy lagt till för ritpussel

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DrawingPuzzleWidget.generated.h"

class UTextureRenderTarget2D;
class UInventoryComponent;
class UImage;

/**
 * Base class for the Drawing puzzle widget.
 *
 * The WBP subclass only needs to provide the Designer hierarchy:
 *   - A CanvasPanel root.
 *   - An Image child named EXACTLY "PaperBackground"  (bound via BindWidget).
 *   - An Image child named EXACTLY "Crosshair"        (bound via BindWidget).
 *
 * Everything else — input, rendering, crosshair movement, A-button to finish —
 * is handled here in C++. The WBP's event graph can be completely empty.
 */
UCLASS(Abstract, Blueprintable)
class SPM_GRUPP3_API UDrawingPuzzleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ===== Bound widgets (must exist in WBP with these names) =====

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PaperBackground;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Crosshair;

	// ===== Editable in WBP defaults =====

	/** Item ID granted to the player when the puzzle is finished. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	FName ResultItemID = TEXT("Drawing");

	/** Resolution of the drawing canvas in pixels (square). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	int32 CanvasSize = 1024;

	/** Line thickness in pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	float LineThickness = 4.f;

	/** Pen color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	FLinearColor LineColor = FLinearColor::Black;

	/** Paper background color (used when clearing). Default transparent so a paper-texture
	 *  Image placed BEHIND PaperBackground in the WBP can shine through. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	FLinearColor BackgroundColor = FLinearColor(1.f, 1.f, 1.f, 0.f);

	/** If true, pushing the right stick UP moves the crosshair UP on screen.
	 *  Set to false if input feels inverted on your project. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle|Input")
	bool bInvertStickY = false;

	/** Speed (UV per second) at full right-stick deflection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	float CrosshairSpeed = 0.6f;

	/** On-screen pixel size of PaperBackground. The Crosshair is positioned within this rect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	FVector2D PaperSizePixels = FVector2D(800.f, 800.f);

	/** Pixel size of the Crosshair widget. Used to center it on the UV target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	FVector2D CrosshairSize = FVector2D(32.f, 32.f);

	/** Right stick magnitude below this is treated as no input (avoids drift). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	float StickDeadzone = 0.15f;

	// ===== Runtime data =====

	UPROPERTY(BlueprintReadOnly, Category = "Drawing Puzzle")
	TObjectPtr<UTextureRenderTarget2D> CanvasTexture;

	// ===== BP-callable utilities (rarely needed; kept for flexibility) =====

	UFUNCTION(BlueprintCallable, Category = "Drawing Puzzle")
	void ClearCanvas();

	UFUNCTION(BlueprintCallable, Category = "Drawing Puzzle")
	void DrawSegment(FVector2D FromUV, FVector2D ToUV);

	UFUNCTION(BlueprintCallable, Category = "Drawing Puzzle")
	void FinishPuzzle();

	UFUNCTION(BlueprintPure, Category = "Drawing Puzzle")
	FVector2D GetRightStickInput() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Disables / re-enables the player pawn's input so right-stick / mouse-look
	 *  doesn't move the camera while the puzzle is open. */
	void SetPlayerInputBlocked(bool bBlocked);

private:
	/** Current crosshair UV in [0,1]² — top-left is (0,0), bottom-right is (1,1). */
	FVector2D CurrentUV = FVector2D(0.5f, 0.5f);

	/** UV from the previous tick — DrawSegment uses this as the segment start. */
	FVector2D PreviousUV = FVector2D(0.5f, 0.5f);

	/** Guards against double-calling FinishPuzzle from both event and polling paths. */
	bool bIsFinishing = false;

	UInventoryComponent* GetPlayerInventory() const;
	void ApplyBrushToPaperBackground();
	void UpdateCrosshairWidgetPosition();
	bool IsConfirmKey(const FKey& Key) const;

	/** Polls the player controller for confirm keys / mouse drawing. Called from NativeTick.
	 *  Bypasses UMG focus so it works even if SetKeyboardFocus didn't stick. */
	bool PollConfirmInput();
	bool PollMouseDrawing();
};
