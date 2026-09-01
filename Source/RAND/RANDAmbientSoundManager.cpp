// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDAmbientSoundManager.h"
#include "RANDDistrictVolume.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "EngineUtils.h"

URANDAmbientSoundManager::URANDAmbientSoundManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f; // Zone changes are a walking-pace event.
}

URANDAmbientSoundManager* URANDAmbientSoundManager::Get(const UObject* WorldContext)
{
	if (!WorldContext) return nullptr;
	const UWorld* World = WorldContext->GetWorld();
	if (!World) return nullptr;
	if (AGameModeBase* GameMode = World->GetAuthGameMode())
	{
		return GameMode->FindComponentByClass<URANDAmbientSoundManager>();
	}
	return nullptr;
}

void URANDAmbientSoundManager::BeginPlay()
{
	Super::BeginPlay();
	BuildProfiles();
}

void URANDAmbientSoundManager::BuildProfiles()
{
	auto Profile = [](float Base, float Crowd, float Traffic, bool bTaxi, bool bTraders)
	{
		FAmbientProfile P;
		P.BaseVolume = Base;
		P.CrowdDensity = Crowd;
		P.TrafficDensity = Traffic;
		P.bHasTaxiRank = bTaxi;
		P.bHasInformalTraders = bTraders;
		return P;
	};

	// Johannesburg -- dense, loud, informal.
	Profiles.Add(EAmbientZone::Hillbrow,     Profile(0.95f, 1.00f, 1.00f, true,  true));
	Profiles.Add(EAmbientZone::MarshallTown, Profile(0.75f, 0.55f, 0.85f, false, true));
	Profiles.Add(EAmbientZone::ParkStation,  Profile(1.00f, 1.00f, 1.00f, true,  true));
	Profiles.Add(EAmbientZone::Maboneng,     Profile(0.45f, 0.30f, 0.30f, false, false));

	// Pretoria -- administrative, quieter, more suburban.
	Profiles.Add(EAmbientZone::PretoriaCBD,  Profile(0.65f, 0.55f, 0.55f, false, false));
	Profiles.Add(EAmbientZone::Arcadia,      Profile(0.40f, 0.30f, 0.30f, false, false));
	Profiles.Add(EAmbientZone::Sunnyside,    Profile(0.80f, 0.80f, 0.55f, false, true));
	Profiles.Add(EAmbientZone::Hatfield,     Profile(0.55f, 0.55f, 0.30f, false, false));
	Profiles.Add(EAmbientZone::Centurion,    Profile(0.55f, 0.30f, 0.85f, false, false));
}

void URANDAmbientSoundManager::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const EAmbientZone Detected = DetectZone();
	if (Detected != CurrentZone)
	{
		const EAmbientZone OldZone = CurrentZone;
		CurrentZone = Detected;
		OnZoneChanged.Broadcast(OldZone, CurrentZone);
	}
}

EAmbientZone URANDAmbientSoundManager::DetectZone() const
{
	UWorld* World = GetWorld();
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!World || !Player) return EAmbientZone::None;

	// Works whether André is on foot or in a vehicle -- both are the player pawn.
	const FVector Location = Player->GetActorLocation();
	for (TActorIterator<ARANDDistrictVolume> It(World); It; ++It)
	{
		if (It->EncompassesPoint(Location))
		{
			return ZoneFromDistrict(It->District);
		}
	}
	return EAmbientZone::None;
}

const FAmbientProfile& URANDAmbientSoundManager::GetProfile(EAmbientZone Zone) const
{
	static const FAmbientProfile Default;
	const FAmbientProfile* Found = Profiles.Find(Zone);
	return Found ? *Found : Default;
}

const FAmbientProfile& URANDAmbientSoundManager::GetCurrentProfile() const
{
	return GetProfile(CurrentZone);
}

EAmbientZone URANDAmbientSoundManager::ZoneFromDistrict(EDistrict District)
{
	switch (District)
	{
	case EDistrict::Hillbrow:     return EAmbientZone::Hillbrow;
	case EDistrict::MarshallTown: return EAmbientZone::MarshallTown;
	case EDistrict::ParkStation:  return EAmbientZone::ParkStation;
	case EDistrict::Maboneng:     return EAmbientZone::Maboneng;
	case EDistrict::PretoriaCBD:  return EAmbientZone::PretoriaCBD;
	case EDistrict::Arcadia:      return EAmbientZone::Arcadia;
	case EDistrict::Sunnyside:    return EAmbientZone::Sunnyside;
	case EDistrict::Hatfield:     return EAmbientZone::Hatfield;
	case EDistrict::Centurion:    return EAmbientZone::Centurion;
	default:                      return EAmbientZone::None;
	}
}
