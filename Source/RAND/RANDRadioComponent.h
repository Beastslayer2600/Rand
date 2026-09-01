// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDRadioComponent.generated.h"

/** In-car radio stations, roughly mapping the Joburg dial. */
UENUM(BlueprintType)
enum class ERadioStation : uint8
{
	Station947	UMETA(DisplayName = "947"),
	YFM			UMETA(DisplayName = "YFM"),
	RSG			UMETA(DisplayName = "RSG"),
	TalkRadio	UMETA(DisplayName = "Talk Radio"),
};

/** One track (or talk segment) in a station's rotation. */
USTRUCT(BlueprintType)
struct FRadioTrack
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Radio") FString StationName;
	UPROPERTY(BlueprintReadOnly, Category = "Radio") FString TrackName;
	UPROPERTY(BlueprintReadOnly, Category = "Radio") FString Artist;

	/** Play length in game-minutes. */
	UPROPERTY(BlueprintReadOnly, Category = "Radio") int32 Duration = 3;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStationChanged, ERadioStation, NewStation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackChanged, const FRadioTrack&, NewTrack);

/**
 * URANDRadioComponent — the car radio, on ARANDVehicle.
 *
 * Four stations, each with a placeholder track list. Tracks advance off the game
 * clock (each track runs for its Duration in game-minutes), and R cycles
 * stations while driving. The HUD shows the current station and track in place
 * of the interaction prompt whenever André is behind the wheel.
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDRadioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDRadioComponent();

	UFUNCTION(BlueprintPure, Category = "Radio") ERadioStation GetStation() const { return Station; }
	UFUNCTION(BlueprintPure, Category = "Radio") const FRadioTrack& GetCurrentTrack() const;

	/** "YFM - Sponono / Kabza De Small" for the HUD. */
	UFUNCTION(BlueprintPure, Category = "Radio") FString GetDisplayString() const;

	/** Human-readable station name. */
	UFUNCTION(BlueprintPure, Category = "Radio") static FString StationToString(ERadioStation InStation);

	UFUNCTION(BlueprintCallable, Category = "Radio") void SetStation(ERadioStation NewStation);

	/** Advance to the next station (wraps). Bound to R while driving. */
	UFUNCTION(BlueprintCallable, Category = "Radio") void CycleStation();

	/** Skip to the next track on the current station. */
	UFUNCTION(BlueprintCallable, Category = "Radio") void NextTrack();

	/** Radio only plays while someone is driving. */
	UFUNCTION(BlueprintCallable, Category = "Radio") void SetPlaying(bool bNewPlaying);

	UFUNCTION(BlueprintPure, Category = "Radio") bool IsPlaying() const { return bPlaying; }

	UPROPERTY(BlueprintAssignable, Category = "Radio") FOnStationChanged OnStationChanged;
	UPROPERTY(BlueprintAssignable, Category = "Radio") FOnTrackChanged OnTrackChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	static constexpr int32 StationCount = 4;

	ERadioStation Station = ERadioStation::Station947;
	bool bPlaying = false;

	/** Per-station playlists, indexed by ERadioStation. */
	TArray<TArray<FRadioTrack>> Playlists;

	/** Current track index per station, so each station keeps its place. */
	int32 TrackIndex[StationCount] = {};

	/** Game-minutes elapsed on the current track. */
	int32 MinutesOnTrack = 0;

	void BuildPlaylists();

	UFUNCTION()
	void HandleMinutePassed(int32 Day, int32 Hour, int32 Minute);
};
