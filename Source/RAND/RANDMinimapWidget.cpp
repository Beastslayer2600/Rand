// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDMinimapWidget.h"
#include "RANDCharacter_NPC.h"
#include "RANDProperty.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FLinearColor BackdropColor(0.03f, 0.05f, 0.04f, 0.55f);
	const FLinearColor PlayerColor(0.20f, 0.85f, 0.35f, 1.0f);
	const FLinearColor NPCColor(0.95f, 0.95f, 0.95f, 1.0f);
	const FLinearColor PropertyColor(0.95f, 0.78f, 0.15f, 1.0f);
}

URANDMinimapWidget::URANDMinimapWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// A small white texture we tint per dot.
	static ConstructorHelpers::FObjectFinder<UTexture2D> WhiteTex(
		TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
	if (WhiteTex.Succeeded())
	{
		DotTexture = WhiteTex.Object;
	}
}

TSharedRef<SWidget> URANDMinimapWidget::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MinimapCanvas"));
		WidgetTree->RootWidget = Canvas;

		// Translucent backdrop filling the minimap area.
		Backdrop = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MinimapBackdrop"));
		StyleDot(Backdrop, BackdropColor);
		if (UCanvasPanelSlot* BackSlot = Canvas->AddChildToCanvas(Backdrop))
		{
			BackSlot->SetPosition(FVector2D(0.0f, 0.0f));
			BackSlot->SetSize(FVector2D(MapDiameter, MapDiameter));
		}

		// Player dot pinned to the centre.
		PlayerDot = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MinimapPlayer"));
		StyleDot(PlayerDot, PlayerColor);
		if (UCanvasPanelSlot* PlayerSlot = Canvas->AddChildToCanvas(PlayerDot))
		{
			PlayerSlot->SetSize(FVector2D(DotSize, DotSize));
			PlayerSlot->SetPosition(FVector2D(MapDiameter * 0.5f - DotSize * 0.5f,
				MapDiameter * 0.5f - DotSize * 0.5f));
		}
	}

	return Super::RebuildWidget();
}

void URANDMinimapWidget::StyleDot(UImage* Dot, const FLinearColor& Color)
{
	if (!Dot)
	{
		return;
	}
	if (DotTexture)
	{
		Dot->SetBrushFromTexture(DotTexture);
	}
	Dot->SetColorAndOpacity(Color);
}

UImage* URANDMinimapWidget::AcquireDot(int32 Index)
{
	while (DotPool.Num() <= Index)
	{
		UImage* NewDot = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		StyleDot(NewDot, NPCColor);
		if (UCanvasPanelSlot* DotSlot = Canvas->AddChildToCanvas(NewDot))
		{
			DotSlot->SetSize(FVector2D(DotSize, DotSize));
		}
		DotPool.Add(NewDot);
	}
	return DotPool[Index];
}

void URANDMinimapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UWorld* World = GetWorld();
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!World || !Player || !Canvas)
	{
		return;
	}

	const FVector PlayerLoc = Player->GetActorLocation();
	const FVector Fwd = Player->GetActorForwardVector();
	const FVector Right = Player->GetActorRightVector();
	const FVector2D Center(MapDiameter * 0.5f, MapDiameter * 0.5f);
	const float Radius = MapDiameter * 0.5f;
	const float HalfDot = DotSize * 0.5f;

	int32 Used = 0;

	// Maps a world location to a dot position; returns false if outside the map.
	auto PlaceDot = [&](const FVector& WorldLoc, const FLinearColor& Color) -> void
	{
		const FVector Rel = WorldLoc - PlayerLoc;
		const float FwdComp = Rel.X * Fwd.X + Rel.Y * Fwd.Y;   // forward = up
		const float RightComp = Rel.X * Right.X + Rel.Y * Right.Y; // right = right

		const float PxRight = RightComp / CMPerPixel;
		const float PxFwd = FwdComp / CMPerPixel;
		if (FMath::Sqrt(PxRight * PxRight + PxFwd * PxFwd) > Radius)
		{
			return; // Outside the circular plot.
		}

		UImage* Dot = AcquireDot(Used++);
		StyleDot(Dot, Color);
		Dot->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Dot->Slot))
		{
			Slot->SetPosition(FVector2D(
				Center.X + PxRight - HalfDot,
				Center.Y - PxFwd - HalfDot));
		}
	};

	for (TActorIterator<ARANDCharacter_NPC> It(World); It; ++It)
	{
		PlaceDot(It->GetActorLocation(), NPCColor);
	}
	for (TActorIterator<ARANDProperty> It(World); It; ++It)
	{
		if (It->bIsOwned)
		{
			PlaceDot(It->GetActorLocation(), PropertyColor);
		}
	}

	// Hide any pool dots not used this frame.
	for (int32 i = Used; i < DotPool.Num(); ++i)
	{
		if (DotPool[i])
		{
			DotPool[i]->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
