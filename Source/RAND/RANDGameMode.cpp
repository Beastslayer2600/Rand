// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDGameMode.h"
#include "RANDCharacter.h"
#include "RANDHUD.h"
#include "TimeComponent.h"
#include "RANDSaveGameManager.h"
#include "RANDMissionManager.h"

ARANDGameMode::ARANDGameMode()
{
	DefaultPawnClass = ARANDCharacter::StaticClass();

	// The player controller spawns this HUD; its BeginPlay builds the UMG
	// widget and binds it to André's health/heat/interaction components.
	HUDClass = ARANDHUD::StaticClass();

	// Authoritative session clock; economy, business accrual, and the HUD all
	// read game time from here rather than running their own timers.
	TimeComponent = CreateDefaultSubobject<URANDTimeComponent>(TEXT("TimeComponent"));

	// Save/load: F5 manual save and an hourly auto-save.
	SaveManager = CreateDefaultSubobject<URANDSaveGameManager>(TEXT("SaveManager"));

	// Mission registry + flow.
	MissionManager = CreateDefaultSubobject<URANDMissionManager>(TEXT("MissionManager"));
}
