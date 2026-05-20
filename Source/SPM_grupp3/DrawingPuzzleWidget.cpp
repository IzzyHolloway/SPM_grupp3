// //Izzy lagt till för ritpussel

#include "DrawingPuzzleWidget.h"
#include "InventoryComponent.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/Canvas.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "InputCoreTypes.h"
#include "GameFramework/Pawn.h"
#include "Framework/Application/SlateApplication.h"

void UDrawingPuzzleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Create the off-screen render target the paper Image will display.
	if (!CanvasTexture)
	{
		const int32 Size = FMath::Clamp(CanvasSize, 64, 4096);
		CanvasTexture = UKismetRenderingLibrary::CreateRenderTarget2D(
			this, Size, Size, ETextureRenderTargetFormat::RTF_RGBA8);
	}

	ClearCanvas();
	ApplyBrushToPaperBackground();

	// Reset crosshair to the center of the paper.
	CurrentUV  = FVector2D(0.5f, 0.5f);
	PreviousUV = CurrentUV;
	UpdateCrosshairWidgetPosition();

	// Make sure the widget can actually take focus (so OnKeyDown fires for gamepad A).
	SetIsFocusable(true);
	SetKeyboardFocus();

	// Block player input so the right stick / mouse-look doesn't move the camera while drawing.
	SetPlayerInputBlocked(true);
}

void UDrawingPuzzleWidget::NativeDestruct()
{
	// Safety: if the widget is removed without FinishPuzzle being called, restore input.
	SetPlayerInputBlocked(false);
	Super::NativeDestruct();
}

void UDrawingPuzzleWidget::SetPlayerInputBlocked(bool bBlocked)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return;

	if (bBlocked)
	{
		Pawn->DisableInput(PC);
	}
	else
	{
		Pawn->EnableInput(PC);
	}
}

void UDrawingPuzzleWidget::ApplyBrushToPaperBackground()
{
	if (!PaperBackground || !CanvasTexture) return;

	FSlateBrush Brush;
	Brush.SetResourceObject(CanvasTexture);
	Brush.ImageSize = PaperSizePixels;
	Brush.DrawAs    = ESlateBrushDrawType::Image;
	PaperBackground->SetBrush(Brush);
}

void UDrawingPuzzleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsFinishing) return;

	// 1) Polling fallback: A / Enter / Space / LMB → finish.
	if (PollConfirmInput())
	{
		FinishPuzzle();
		return;
	}

	// 2) Mouse drawing (works even when widget didn't get UI focus).
	if (PollMouseDrawing())
	{
		return; // Mouse moved this tick → don't also process stick.
	}

	// 3) Gamepad right stick.
	const FVector2D Stick = GetRightStickInput();
	const FVector2D Delta(Stick.X, bInvertStickY ? -Stick.Y : Stick.Y);

	// Below deadzone: don't even draw a 0-length segment. Saves a render pass per tick.
	if (Delta.SizeSquared() < StickDeadzone * StickDeadzone)
	{
		return;
	}

	PreviousUV = CurrentUV;

	FVector2D Next = CurrentUV + Delta * CrosshairSpeed * InDeltaTime;
	Next.X = FMath::Clamp(Next.X, 0.f, 1.f);
	Next.Y = FMath::Clamp(Next.Y, 0.f, 1.f);
	CurrentUV = Next;

	DrawSegment(PreviousUV, CurrentUV);
	UpdateCrosshairWidgetPosition();
}

bool UDrawingPuzzleWidget::PollConfirmInput()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return false;

	return PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom)
	    || PC->WasInputKeyJustPressed(EKeys::Enter)
	    || PC->WasInputKeyJustPressed(EKeys::SpaceBar)
	    || PC->WasInputKeyJustPressed(EKeys::LeftMouseButton);
}

bool UDrawingPuzzleWidget::PollMouseDrawing()
{
	if (!PaperBackground) return false;
	if (!FSlateApplication::IsInitialized()) return false;

	// Use Slate's cursor position — it's in absolute screen pixels, which matches
	// the coordinate system used by FGeometry::AbsoluteToLocal().
	// (APlayerController::GetMousePosition returns viewport-local pixels, which
	//  is the WRONG space and was the cause of mouse drawing not working.)
	const FVector2D ScreenPos = FSlateApplication::Get().GetCursorPos();

	const FGeometry  PaperGeom  = PaperBackground->GetCachedGeometry();
	const FVector2D  LocalMouse = PaperGeom.AbsoluteToLocal(ScreenPos);
	const FVector2D  PaperSize  = PaperGeom.GetLocalSize();

	if (PaperSize.X < 1.f || PaperSize.Y < 1.f) return false;

	// Only react when the cursor is actually inside the paper rectangle.
	if (LocalMouse.X < 0.f || LocalMouse.X > PaperSize.X ||
	    LocalMouse.Y < 0.f || LocalMouse.Y > PaperSize.Y)
	{
		return false;
	}

	FVector2D NewUV(LocalMouse.X / PaperSize.X, LocalMouse.Y / PaperSize.Y);
	NewUV.X = FMath::Clamp(NewUV.X, 0.f, 1.f);
	NewUV.Y = FMath::Clamp(NewUV.Y, 0.f, 1.f);

	// No movement → don't waste a render pass.
	if (NewUV.Equals(CurrentUV, 0.001f)) return false;

	PreviousUV = CurrentUV;
	CurrentUV  = NewUV;

	DrawSegment(PreviousUV, CurrentUV);
	UpdateCrosshairWidgetPosition();
	return true;
}

void UDrawingPuzzleWidget::UpdateCrosshairWidgetPosition()
{
	if (!Crosshair) return;

	// Use the PaperBackground's ACTUAL on-screen size (after anchors / layout)
	// so the crosshair tracks the rendered paper exactly, regardless of WBP design size.
	FVector2D PaperSize = PaperSizePixels; // fallback
	if (PaperBackground)
	{
		const FVector2D Cached = PaperBackground->GetCachedGeometry().GetLocalSize();
		if (Cached.X > 1.f && Cached.Y > 1.f)
		{
			PaperSize = Cached;
		}
	}

	const FVector2D CenterPx = CurrentUV * PaperSize;

	// Preferred: change the layout position via the CanvasPanel slot.
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Crosshair->Slot))
	{
		// Force anchor=top-left + alignment=center so SetPosition places the
		// crosshair's CENTER at CenterPx regardless of what the WBP author set.
		// This avoids relying on the CrosshairSize UPROPERTY matching the actual image size.
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(CenterPx);
	}
	else
	{
		// Fallback (Crosshair not in a CanvasPanel): use render translation.
		Crosshair->SetRenderTranslation(CenterPx - CrosshairSize * 0.5f);
	}
}

bool UDrawingPuzzleWidget::IsConfirmKey(const FKey& Key) const
{
	return Key == EKeys::Gamepad_FaceButton_Bottom  // A on Xbox / Cross on PS
	    || Key == EKeys::Enter
	    || Key == EKeys::SpaceBar;
}

FReply UDrawingPuzzleWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (IsConfirmKey(InKeyEvent.GetKey()))
	{
		FinishPuzzle();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UDrawingPuzzleWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		FinishPuzzle();
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UDrawingPuzzleWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!PaperBackground)
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}

	// Convert absolute screen-space mouse position to PaperBackground-local pixels,
	// then to UV in [0,1]² so it matches the gamepad-stick coordinate system.
	const FGeometry  PaperGeom  = PaperBackground->GetCachedGeometry();
	const FVector2D  LocalMouse = PaperGeom.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	const FVector2D  PaperSize  = PaperGeom.GetLocalSize();

	if (PaperSize.X < 1.f || PaperSize.Y < 1.f)
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}

	// Only react when the cursor is actually inside the paper rectangle.
	if (LocalMouse.X < 0.f || LocalMouse.X > PaperSize.X ||
	    LocalMouse.Y < 0.f || LocalMouse.Y > PaperSize.Y)
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}

	FVector2D NewUV(LocalMouse.X / PaperSize.X, LocalMouse.Y / PaperSize.Y);
	NewUV.X = FMath::Clamp(NewUV.X, 0.f, 1.f);
	NewUV.Y = FMath::Clamp(NewUV.Y, 0.f, 1.f);

	PreviousUV = CurrentUV;
	CurrentUV  = NewUV;

	DrawSegment(PreviousUV, CurrentUV);
	UpdateCrosshairWidgetPosition();

	return FReply::Handled();
}

void UDrawingPuzzleWidget::ClearCanvas()
{
	if (!CanvasTexture) return;
	UKismetRenderingLibrary::ClearRenderTarget2D(this, CanvasTexture, BackgroundColor);
}

void UDrawingPuzzleWidget::DrawSegment(FVector2D FromUV, FVector2D ToUV)
{
	if (!CanvasTexture) return;

	FromUV.X = FMath::Clamp(FromUV.X, 0.f, 1.f);
	FromUV.Y = FMath::Clamp(FromUV.Y, 0.f, 1.f);
	ToUV.X   = FMath::Clamp(ToUV.X,   0.f, 1.f);
	ToUV.Y   = FMath::Clamp(ToUV.Y,   0.f, 1.f);

	if (FromUV.Equals(ToUV, 0.001f))
	{
		return;
	}

	UCanvas* Canvas = nullptr;
	FVector2D Size(0, 0);
	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, CanvasTexture, Canvas, Size, Context);

	if (Canvas)
	{
		Canvas->K2_DrawLine(FromUV * Size, ToUV * Size, LineThickness, LineColor);
	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
}

void UDrawingPuzzleWidget::FinishPuzzle()
{
	// Guard against double-trigger from polling + event path.
	if (bIsFinishing) return;
	bIsFinishing = true;

	// Re-enable player input BEFORE granting the item, so any reaction (like a
	// "You created X" notification widget) starts from a clean input state.
	SetPlayerInputBlocked(false);

	// Granting the item broadcasts OnItemCrafted, which the player character
	// listens to and uses to spawn the existing "You created X" notification widget.
	if (UInventoryComponent* Inv = GetPlayerInventory())
	{
		Inv->AddCraftedItem(ResultItemID);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("DrawingPuzzleWidget: no InventoryComponent on the player character — item not granted."));
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
		PC->bShowMouseCursor = false;
	}

	RemoveFromParent();
}

FVector2D UDrawingPuzzleWidget::GetRightStickInput() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return FVector2D::ZeroVector;

	float X = 0.f;
	float Y = 0.f;
	PC->GetInputAnalogStickState(EControllerAnalogStick::CAS_RightStick, X, Y);
	return FVector2D(X, Y);
}

UInventoryComponent* UDrawingPuzzleWidget::GetPlayerInventory() const
{
	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0);
	return Character ? Character->FindComponentByClass<UInventoryComponent>() : nullptr;
}
