
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DrawingPuzzleWidget.generated.h"

class UTextureRenderTarget2D;
class UInventoryComponent;
class UImage;


UCLASS(Abstract, Blueprintable)
class SPM_GRUPP3_API UDrawingPuzzleWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PaperBackground;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Crosshair;

	// Puzzle settings     
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	FName ResultItemID = TEXT("Drawing");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	int32 CanvasSize = 1024;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	float LineThickness = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	FLinearColor LineColor = FLinearColor::Black;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	FLinearColor BackgroundColor = FLinearColor(1.f, 1.f, 1.f, 0.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle|Input")
	bool bInvertStickY = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	float CrosshairSpeed = 0.6f;
	
	// UI sizes to correctly map the mouse/crosshair coordinates to our Render Target.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	FVector2D PaperSizePixels = FVector2D(800.f, 800.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	FVector2D CrosshairSize = FVector2D(32.f, 32.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drawing Puzzle")
	float StickDeadzone = 0.15f;
	
	// The surface we're painting on.
	UPROPERTY(BlueprintReadOnly, Category = "Drawing Puzzle")
	TObjectPtr<UTextureRenderTarget2D> CanvasTexture;

	UFUNCTION(BlueprintCallable, Category = "Drawing Puzzle")
	void ClearCanvas();
	
	// Draws a line from the previous frame's position to the current one. 
	//I use UVs (0 to 1) so the code works regardless of the player's screen resolution.
	UFUNCTION(BlueprintCallable, Category = "Drawing Puzzle")
	void DrawSegment(FVector2D FromUV, FVector2D ToUV);

	UFUNCTION(BlueprintCallable, Category = "Drawing Puzzle")
	void FinishPuzzle();

	UFUNCTION(BlueprintPure, Category = "Drawing Puzzle")
	FVector2D GetRightStickInput() const;

protected:
	// Setup and cleanup
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	// Tick loop to handle gamepad crosshair movement.
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	// Input events for mouse and keyboard.
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	// Ensures the player character doesn't walk around in the background while the puzzle is active.
	void SetPlayerInputBlocked(bool bBlocked);

private:
	
	// Keeps track of where we are drawing right now, and where we were last frame.
	// Required to draw lines (DrawSegment) instead of just individual dots.
	FVector2D CurrentUV = FVector2D(0.5f, 0.5f);
	FVector2D PreviousUV = FVector2D(0.5f, 0.5f);

	// Prevents the player from accidentally triggering FinishPuzzle multiple times.
	bool bIsFinishing = false;

	// //Izzy lagt till för bug-fix: musen ska inte rita om den inte rört sig.
	// Tracks the previous mouse screen position so we can ignore frames where the
	// cursor is stationary (otherwise the mouse logic fights joystick input and
	// also draws a stray line on the first frame).
	FVector2D LastMouseScreenPos = FVector2D::ZeroVector;
	bool bHasInitialMousePos = false;

	UInventoryComponent* GetPlayerInventory() const;
	void ApplyBrushToPaperBackground();
	void UpdateCrosshairWidgetPosition();
	
	// Input checks
	bool IsConfirmKey(const FKey& Key) const;
	bool PollConfirmInput();
	bool PollMouseDrawing();
};
