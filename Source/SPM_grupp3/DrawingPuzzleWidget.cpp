// //Izzy lagt till för ritpussel

#include "DrawingPuzzleWidget.h"
#include "InventoryComponent.h"
#include "Engine/Canvas.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UDrawingPuzzleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!CanvasTexture)
	{
		const int32 Size = FMath::Clamp(CanvasSize, 64, 4096);
		CanvasTexture = UKismetRenderingLibrary::CreateRenderTarget2D(
			this, Size, Size, ETextureRenderTargetFormat::RTF_RGBA8);
	}

	ClearCanvas();
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
		// No movement — skip to avoid wasting a render pass.
		return;
	}

	UCanvas* Canvas = nullptr;
	FVector2D Size(0, 0);
	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, CanvasTexture, Canvas, Size, Context);

	if (Canvas)
	{
		const FVector2D From = FromUV * Size;
		const FVector2D To   = ToUV   * Size;
		Canvas->K2_DrawLine(From, To, LineThickness, LineColor);
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
