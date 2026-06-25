// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RANDMinimapWidget.generated.h"

class UCanvasPanel;
class UImage;
class UTexture2D;

/**
 * URANDMinimapWidget — a top-right minimap built with the UMG canvas API.
 *
 * André sits at the centre; nearby NPCs are white dots and owned properties are
 * gold dots. The map rotates with André's facing (his heading points up). Scale
 * is 1 pixel = 2 metres, and the plot is clipped to a 200px circle by culling
 * dots beyond the radius (no mask texture needed for a code-only project).
 */
UCLASS()
class RAND_API URANDMinimapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	URANDMinimapWidget(const FObjectInitializer& ObjectInitializer);

	/** Diameter of the minimap in pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float MapDiameter = 200.0f;

	/** World centimetres represented by one minimap pixel (200 = 1px : 2m). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float CMPerPixel = 200.0f;

	/** Size (px) of each plotted dot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float DotSize = 8.0f;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY()
	TObjectPtr<UCanvasPanel> Canvas;

	UPROPERTY()
	TObjectPtr<UImage> Backdrop;

	UPROPERTY()
	TObjectPtr<UImage> PlayerDot;

	/** Reusable pool of dots for NPCs/properties. */
	UPROPERTY()
	TArray<TObjectPtr<UImage>> DotPool;

private:
	UPROPERTY()
	TObjectPtr<UTexture2D> DotTexture;

	/** Returns pool dot at Index, creating and registering it on first use. */
	UImage* AcquireDot(int32 Index);

	/** Configures an image as a solid tinted square of DotSize. */
	void StyleDot(UImage* Dot, const FLinearColor& Color);
};
