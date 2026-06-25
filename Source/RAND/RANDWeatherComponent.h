// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDWeatherComponent.generated.h"

class AExponentialHeightFog;
class APostProcessVolume;

/** Weather states, from clear skies to a full Highveld thunderstorm. */
UENUM(BlueprintType)
enum class EWeatherState : uint8
{
	Clear		UMETA(DisplayName = "Clear"),
	Overcast	UMETA(DisplayName = "Overcast"),
	LightRain	UMETA(DisplayName = "Light Rain"),
	HeavyRain	UMETA(DisplayName = "Heavy Rain"),
	Thunderstorm	UMETA(DisplayName = "Thunderstorm"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherChanged, EWeatherState, OldState, EWeatherState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadSheddingChanged, int32, EskomStage);

/**
 * URANDWeatherComponent — lives on ARANDGameMode. Models Johannesburg's summer
 * afternoon thunderstorms and Eskom load shedding off the game clock.
 *
 * Weather: every game hour between 14:00 and 17:00 there's a 40% chance a storm
 * rolls in (-> Thunderstorm); otherwise the weather eases one step toward Clear.
 * Each state drives the level's height fog density and post-process volume
 * (saturation/exposure) for a wetter, darker look.
 *
 * Load shedding: once per game day an Eskom stage (0-8) is rolled. Stage 4+
 * triggers lights-off — every point/spot light in the level (street lights,
 * building lights) is switched off until the stage drops below the threshold.
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDWeatherComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDWeatherComponent();

	// --- Weather ------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Weather")
	EWeatherState GetWeather() const { return CurrentWeather; }

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeather(EWeatherState NewState);

	/** 0.0-1.0 chance of a storm per hour during the afternoon window. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AfternoonStormChance = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0", ClampMax = "23"))
	int32 StormWindowStartHour = 14;

	/** Exclusive end hour of the storm window. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0", ClampMax = "24"))
	int32 StormWindowEndHour = 17;

	UPROPERTY(BlueprintAssignable, Category = "Weather")
	FOnWeatherChanged OnWeatherChanged;

	// --- Load shedding ------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Weather|LoadShedding")
	int32 GetEskomStage() const { return EskomStage; }

	UFUNCTION(BlueprintCallable, Category = "Weather|LoadShedding")
	void SetEskomStage(int32 NewStage);

	UFUNCTION(BlueprintPure, Category = "Weather|LoadShedding")
	bool IsLoadShedding() const { return EskomStage >= LoadSheddingThreshold; }

	/** Eskom stage at or above which the lights go off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|LoadShedding", meta = (ClampMin = "0", ClampMax = "8"))
	int32 LoadSheddingThreshold = 4;

	UPROPERTY(BlueprintAssignable, Category = "Weather|LoadShedding")
	FOnLoadSheddingChanged OnLoadSheddingChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	EWeatherState CurrentWeather = EWeatherState::Clear;
	int32 EskomStage = 0;

	TWeakObjectPtr<AExponentialHeightFog> Fog;
	TWeakObjectPtr<APostProcessVolume> PostProcess;

	UFUNCTION()
	void HandleHourPassed(int32 Day, int32 Hour);

	UFUNCTION()
	void HandleDayPassed(int32 Day);

	void CacheSceneActors();
	void ApplyWeatherVisuals();
	void ApplyLoadShedding();

	/** One step toward Clear (Thunderstorm -> ... -> Clear). */
	static EWeatherState DecayedState(EWeatherState State);
};
