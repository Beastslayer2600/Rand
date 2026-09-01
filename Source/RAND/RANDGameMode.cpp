// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDGameMode.h"
#include "RANDCharacter.h"
#include "RANDHUD.h"
#include "TimeComponent.h"
#include "RANDSaveGameManager.h"
#include "RANDMissionManager.h"
#include "RANDWeatherComponent.h"
#include "RANDCampaignDirector.h"
#include "RANDWantedResponseComponent.h"
#include "RANDSARSComponent.h"
#include "RANDCheatComponent.h"
#include "RANDSideHustleComponent.h"
#include "RANDWorldSeeder.h"
#include "RANDAct3Director.h"
#include "RANDNewsTicker.h"
#include "RANDAmbientSoundManager.h"

ARANDGameMode::ARANDGameMode()
{
	DefaultPawnClass = ARANDCharacter::StaticClass();
	HUDClass = ARANDHUD::StaticClass();
	TimeComponent = CreateDefaultSubobject<URANDTimeComponent>(TEXT("TimeComponent"));
	SaveManager = CreateDefaultSubobject<URANDSaveGameManager>(TEXT("SaveManager"));
	MissionManager = CreateDefaultSubobject<URANDMissionManager>(TEXT("MissionManager"));
	WeatherComponent = CreateDefaultSubobject<URANDWeatherComponent>(TEXT("WeatherComponent"));
	CampaignDirector = CreateDefaultSubobject<URANDCampaignDirector>(TEXT("CampaignDirector"));
	WantedResponse   = CreateDefaultSubobject<URANDWantedResponseComponent>(TEXT("WantedResponse"));
	SARSComponent    = CreateDefaultSubobject<URANDSARSComponent>(TEXT("SARSComponent"));
	CheatComponent   = CreateDefaultSubobject<URANDCheatComponent>(TEXT("CheatComponent"));
	SideHustle       = CreateDefaultSubobject<URANDSideHustleComponent>(TEXT("SideHustle"));
	WorldSeeder      = CreateDefaultSubobject<URANDWorldSeeder>(TEXT("WorldSeeder"));
	Act3Director     = CreateDefaultSubobject<URANDAct3Director>(TEXT("Act3Director"));
	NewsTicker       = CreateDefaultSubobject<URANDNewsTicker>(TEXT("NewsTicker"));
	AmbientSound     = CreateDefaultSubobject<URANDAmbientSoundManager>(TEXT("AmbientSound"));
}
