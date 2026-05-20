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

	// Take keyboard focus so NativeOnKeyDown fires for the A button / Enter / Space.
	SetKeyboardFocus();
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

	// Read right stick. bInvertStickY toggles whether up-on-stick = up-on-screen.
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
	const FVector2D TopLeft  = CenterPx - CrosshairSize * 0.5f;

	// Preferred: change the layout position via the CanvasPanel slot.
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Crosshair->Slot))
	{
		CanvasSlot->SetPosition(TopLeft);
	}
	else
	{
		// Fallback (Crosshair not in a CanvasPanel): use render translation.
		Crosshair->SetRenderTranslation(TopLeft);
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
