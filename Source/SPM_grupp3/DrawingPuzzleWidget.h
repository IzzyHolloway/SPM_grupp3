// //Izzy lagt till för ritpussel

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DrawingPuzzleWidget.generated.h"

class UTextureRenderTarget2D;
class UInventoryComponent;

/**
 * Base class for the Drawing puzzle widget.
 *
 * Subclass this in UMG (WBP_DrawingPuzzle) and:
 *   - Add an Image bound to CanvasTexture (paper background that shows the drawing).
 *   - Add an Image acting as a crosshair, positioned each tick.
 *   - Read input (right stick via GetRightStickInput, or mouse via GetMousePosition),
 *     move the crosshair, and call DrawSegment(prevUV, newUV) every tick.
 *   - On A press (or left click), call FinishPuzzle.
 *
 * Lifecycle:
 *   - NativeConstruct creates the CanvasTexture and clears it.
 *   - FinishPuzzle adds ResultItemID to the player's inventory and removes the widget.
 */
UCLASS(Abstract, Blueprintable)
class SPM_GRUPP3_API UDrawingPuzzleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Item ID granted to the player when FinishPuzzle is called. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	FName ResultItemID = TEXT("Drawing");

	/** Resolution of the drawing canvas, in pixels (square). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	int32 CanvasSize = 1024;

	/** Line thickness in pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	float LineThickness = 4.f;

	/** Pen color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	FLinearColor LineColor = FLinearColor::Black;

	/** Paper background color (used when clearing). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	FLinearColor BackgroundColor = FLinearColor::White;

	/** Render target the paper Image should display. Generated in NativeConstruct. */
	UPROPERTY(BlueprintReadOnly, Category = "Drawing Puzzle")
	TObjectPtr<UTextureRenderTarget2D> CanvasTexture;

	/** Resets the drawing to BackgroundColor. */
	UFUNCTION(BlueprintCallable, Category = "Drawing Puzzle")
	void ClearCanvas();

	/**
	 * Draws a line segment between two UV positions on the canvas.
	 * Both inputs are in [0,1] where (0,0) is top-left and (1,1) is bottom-right.
	 * Call from BP each tick: FromUV = previous crosshair UV, ToUV = current.
	 */
	UFUNCTION(BlueprintCallable, Category = "Drawing Puzzle")
	void DrawSegment(FVector2D FromUV, FVector2D ToUV);

	/**
	 * Completes the puzzle: grants ResultItemID to the player's InventoryComponent,
	 * restores Game-only input mode, hides the cursor, and removes this widget.
	 */
	UFUNCTION(BlueprintCallable, Category = "Drawing Puzzle")
	void FinishPuzzle();

	/** Returns the right analog stick as (X,Y) in [-1,1]. (0,0) if no controller / no PC. */
	UFUNCTION(BlueprintPure, Category = "Drawing Puzzle")
	FVector2D GetRightStickInput() const;

protected:
	virtual void NativeConstruct() override;

private:
	UInventoryComponent* GetPlayerInventory() const;
};
