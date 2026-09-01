// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDLanguageTypes.h"
#include "RANDAmbientSoundManager.generated.h"

/** Ambient zones, one per district (mirrors EDistrict). */
UENUM(BlueprintType)
enum class EAmbientZone : uint8
{
	None			UMETA(DisplayName = "None"),
	Hillbrow		UMETA(DisplayName = "Hillbrow"),
	MarshallTown	UMETA(DisplayName = "Marshalltown"),
	ParkStation		UMETA(DisplayName = "Park Station"),
	Maboneng		UMETA(DisplayName = "Maboneng"),
	PretoriaCBD		UMETA(DisplayName = "Pretoria CBD"),
	Arcadia			UMETA(DisplayName = "Arcadia"),
	Sunnyside		UMETA(DisplayName = "Sunnyside"),
	Hatfield		UMETA(DisplayName = "Hatfield"),
	Centurion		UMETA(DisplayName = "Centurion"),
};

/**
 * FAmbientProfile — the soundscape character of a zone. Densities are 0-1
 * normalised weights that an audio layer will later map onto loop gain and
 * one-shot frequency.
 */
USTRUCT(BlueprintType)
struct FAmbientProfile
{
	GENERATED_BODY()

	/** Overall bed level for the zone (0-1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience") float BaseVolume = 0.5f;

	/** Voices, footfall, street chatter (0-1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience") float CrowdDensity = 0.5f;

	/** Engines, hooters, minibus taxis (0-1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience") float TrafficDensity = 0.5f;

	/** Rank whistles, shouted destinations, sliding doors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience") bool bHasTaxiRank = false;

	/** Pavement traders: braziers, radios, hawker calls. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience") bool bHasInformalTraders = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneChanged, EAmbientZone, OldZone, EAmbientZone, NewZone);

/**
 * URANDAmbientSoundManager — data layer for the city soundscape, on ARANDGameMode.
 *
 * Holds an FAmbientProfile per district and tracks which ARANDDistrictVolume
 * André is standing in, broadcasting OnZoneChanged as he crosses between them.
 * No audio assets are played yet: this exists so the audio pass can bind loops
 * and one-shots to profiles that are already correct per neighbourhood.
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDAmbientSoundManager : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDAmbientSoundManager();

	/** Resolves the manager from the active game mode. */
	static URANDAmbientSoundManager* Get(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "Ambience") EAmbientZone GetCurrentZone() const { return CurrentZone; }

	/** Profile for a zone; a default-constructed profile for None/unknown. */
	UFUNCTION(BlueprintPure, Category = "Ambience") const FAmbientProfile& GetProfile(EAmbientZone Zone) const;

	/** Profile for the zone André is currently in. */
	UFUNCTION(BlueprintPure, Category = "Ambience") const FAmbientProfile& GetCurrentProfile() const;

	/** Maps a district to its matching ambient zone. */
	UFUNCTION(BlueprintPure, Category = "Ambience") static EAmbientZone ZoneFromDistrict(EDistrict District);

	UPROPERTY(BlueprintAssignable, Category = "Ambience") FOnZoneChanged OnZoneChanged;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	EAmbientZone CurrentZone = EAmbientZone::None;

	UPROPERTY()
	TMap<EAmbientZone, FAmbientProfile> Profiles;

	void BuildProfiles();

	/** Which district volume contains André right now (None if outdoors of all). */
	EAmbientZone DetectZone() const;
};
