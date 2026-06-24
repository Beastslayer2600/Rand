// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RANDHUD.generated.h"

class URANDHUDWidget;

/**
 * ARANDHUD — creates the UMG HUD widget on BeginPlay, adds it to the viewport,
 * and binds it to the local player's ARANDCharacter so the heat bars, health
 * bar, and interaction prompt track the player's components.
 *
 * The widget class defaults to URANDHUDWidget (built in C++) but can be
 * overridden with a designer WBP subclass via HUDWidgetClass.
 */
UCLASS()
class RAND_API ARANDHUD : public AHUD
{
	GENERATED_BODY()

public:
	ARANDHUD();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Widget class to instantiate. Defaults to URANDHUDWidget. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<URANDHUDWidget> HUDWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<URANDHUDWidget> HUDWidget;
};
