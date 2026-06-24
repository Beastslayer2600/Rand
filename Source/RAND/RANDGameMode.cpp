// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDGameMode.h"
#include "RANDCharacter.h"
#include "RANDHUD.h"

ARANDGameMode::ARANDGameMode()
{
	DefaultPawnClass = ARANDCharacter::StaticClass();

	// The player controller spawns this HUD; its BeginPlay builds the UMG
	// widget and binds it to André's health/heat/interaction components.
	HUDClass = ARANDHUD::StaticClass();
}
