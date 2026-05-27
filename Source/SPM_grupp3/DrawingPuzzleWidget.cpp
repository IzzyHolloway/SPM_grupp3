
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

	// Safty: If CanvasTexture is missing in the Blueprint, create one dynamically.
	// Size is clamped to prevent memory crashes from accidental typos.
	if (!CanvasTexture)
	{
		const int32 Size = FMath::Clamp(CanvasSize, 64, 4096);
		CanvasTexture = UKismetRenderingLibrary::CreateRenderTarget2D(
			this, Size, Size, ETextureRenderTargetFormat::RTF_RGBA8);
	}

	ClearCanvas();
	ApplyBrushToPaperBackground();

	// Starts the pen right in the middle of the paper (0.5, 0.5 is the center in UV space)
	CurrentUV  = FVector2D(0.5f, 0.5f);
	PreviousUV = CurrentUV;
	UpdateCrosshairWidgetPosition();

	// Makes sure this widget intercepts player input immediately
	SetIsFocusable(true);
	SetKeyboardFocus();

	// Stops the player character from walking around while doing the puzzle
	SetPlayerInputBlocked(true);
}

void UDrawingPuzzleWidget::NativeDestruct()
{
	// Gives control back to the player when the UI is closed!
	SetPlayerInputBlocked(false);
	Super::NativeDestruct();
}

void UDrawingPuzzleWidget::SetPlayerInputBlocked(bool bBlocked)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return;

	//Freezes the player character
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

	// Wraps the UImage in a SlateBrush
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

	// Check if the player pressed A (Done)
	if (PollConfirmInput())
	{
		FinishPuzzle();
		return;
	}

	// Trys to draw with the mouse. If it succeeds, it skips the gamepad logic
	if (PollMouseDrawing())
	{
		return; 
	}


	const FVector2D Stick = GetRightStickInput();
	const FVector2D Delta(Stick.X, bInvertStickY ? -Stick.Y : Stick.Y);

	// Ignores tiny stick drifts
	if (Delta.SizeSquared() < StickDeadzone * StickDeadzone)
	{
		return;
	}

	PreviousUV = CurrentUV;

	// Calculates where the crosshair should move this frame based on stick input and speed.
	// Clamp between 0.0 and 1.0 to keep the pen on the paper.
	FVector2D Next = CurrentUV + Delta * CrosshairSpeed * InDeltaTime;
	Next.X = FMath::Clamp(Next.X, 0.f, 1.f);
	Next.Y = FMath::Clamp(Next.Y, 0.f, 1.f);
	CurrentUV = Next;

	// Actually draw the line and visually move the UI crosshair
	DrawSegment(PreviousUV, CurrentUV);
	UpdateCrosshairWidgetPosition();
}

bool UDrawingPuzzleWidget::PollConfirmInput()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return false;

	// Hardcoded confirm keys
	return PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom)
	    || PC->WasInputKeyJustPressed(EKeys::Enter)
	    || PC->WasInputKeyJustPressed(EKeys::SpaceBar)
	    || PC->WasInputKeyJustPressed(EKeys::LeftMouseButton);
}

bool UDrawingPuzzleWidget::PollMouseDrawing()
{
	if (!PaperBackground) return false;
	if (!FSlateApplication::IsInitialized()) return false;

	// Gets the global screen position of the mouse
	const FVector2D ScreenPos = FSlateApplication::Get().GetCursorPos();

	//and translate it into local widget coordinates for the Paper widget
	const FGeometry  PaperGeom  = PaperBackground->GetCachedGeometry();
	const FVector2D  LocalMouse = PaperGeom.AbsoluteToLocal(ScreenPos);
	const FVector2D  PaperSize  = PaperGeom.GetLocalSize();

	if (PaperSize.X < 1.f || PaperSize.Y < 1.f) return false;

	//Dont draw if the mouse is outside the paper boundaries
	if (LocalMouse.X < 0.f || LocalMouse.X > PaperSize.X ||
	    LocalMouse.Y < 0.f || LocalMouse.Y > PaperSize.Y)
	{
		return false;
	}

	// Convert local pixels to UV space (0.0 to 1.0)
	FVector2D NewUV(LocalMouse.X / PaperSize.X, LocalMouse.Y / PaperSize.Y);
	NewUV.X = FMath::Clamp(NewUV.X, 0.f, 1.f);
	NewUV.Y = FMath::Clamp(NewUV.Y, 0.f, 1.f);


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

	
	FVector2D PaperSize = PaperSizePixels; 
	if (PaperBackground)
	{
		const FVector2D Cached = PaperBackground->GetCachedGeometry().GetLocalSize();
		if (Cached.X > 1.f && Cached.Y > 1.f)
		{
			PaperSize = Cached;
		}
	}
	// Converts 0-1 UV coordinate back into UI pixels so it can place the Crosshair image
	const FVector2D CenterPx = CurrentUV * PaperSize;

	
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Crosshair->Slot))
	{
		
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(CenterPx);
	}
	else
	{
		
		Crosshair->SetRenderTranslation(CenterPx - CrosshairSize * 0.5f);
	}
}

bool UDrawingPuzzleWidget::IsConfirmKey(const FKey& Key) const
{
	return Key == EKeys::Gamepad_FaceButton_Bottom 
	    || Key == EKeys::Enter
	    || Key == EKeys::SpaceBar;
}

FReply UDrawingPuzzleWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Catch the confirm keys through the UI system
	if (IsConfirmKey(InKeyEvent.GetKey()))
	{
		FinishPuzzle();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UDrawingPuzzleWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	//Left clicking finishes the puzzle immediately
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

	
	const FGeometry  PaperGeom  = PaperBackground->GetCachedGeometry();
	const FVector2D  LocalMouse = PaperGeom.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	const FVector2D  PaperSize  = PaperGeom.GetLocalSize();

	if (PaperSize.X < 1.f || PaperSize.Y < 1.f)
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}

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

	// Double check we aren't trying to draw outside the texture
	FromUV.X = FMath::Clamp(FromUV.X, 0.f, 1.f);
	FromUV.Y = FMath::Clamp(FromUV.Y, 0.f, 1.f);
	ToUV.X   = FMath::Clamp(ToUV.X,   0.f, 1.f);
	ToUV.Y   = FMath::Clamp(ToUV.Y,   0.f, 1.f);

	if (FromUV.Equals(ToUV, 0.001f))
	{
		return;
	}

	const float MaxSegmentLength = 0.1f;
	if (FVector2D::Distance(FromUV, ToUV) > MaxSegmentLength)
	{
		return;
	}

	UCanvas* Canvas = nullptr;
	FVector2D Size(0, 0);
	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, CanvasTexture, Canvas, Size, Context);

	if (Canvas)
	{
		// Converts UVs back into texture-pixel sizes before drawing
		Canvas->K2_DrawLine(FromUV * Size, ToUV * Size, LineThickness, LineColor);
	}
	// Applys the drawing to the texture
	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
}

void UDrawingPuzzleWidget::FinishPuzzle()
{
	if (bIsFinishing) return;
	bIsFinishing = true;

	
	SetPlayerInputBlocked(false); // Lets the player move again

	
	if (UInventoryComponent* Inv = GetPlayerInventory())
	{
		Inv->AddCraftedItem(ResultItemID);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("DrawingPuzzleWidget: no InventoryComponent on the player character — item not granted."));
	}

	// Clean up the UI state
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
		PC->bShowMouseCursor = false;
	}

	// Destroys the widget
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
