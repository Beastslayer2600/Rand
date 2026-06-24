// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDGameMode.h"
#include "RANDCharacter.h"
#include "RANDHUD.h"
#include "TimeComponent.h"

ARANDGameMode::ARANDGameMode()
{
	DefaultPawnClass = ARANDCharacter::StaticClass();

	// The player controller spawns this HUD; its BeginPlay builds the UMG
	// widget and binds it to André's health/heat/interaction components.
	HUDClass = ARANDHUD::StaticClass();

	// Authoritative session clock; economy, business accrual, and the HUD all
	// read game time from here rather than running their own timers.
	TimeComponent = CreateDefaultSubobject<URANDTimeComponent>(TEXT("TimeComponent"));
}
