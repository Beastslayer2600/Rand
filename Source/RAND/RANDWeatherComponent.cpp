// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDWeatherComponent.h"
#include "TimeComponent.h"

#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/Light.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Components/LightComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
	// Per-state height fog density and post-process look.
	struct FWeatherLook
	{
		float FogDensity;
		float Saturation;
		float ExposureBias;
	};

	FWeatherLook LookFor(EWeatherState State)
	{
		switch (State)
		{
		case EWeatherState::Overcast:     return { 0.030f, 0.90f, -0.30f };
		case EWeatherState::LightRain:    return { 0.050f, 0.85f, -0.50f };
		case EWeatherState::HeavyRain:    return { 0.080f, 0.80f, -0.80f };
		case EWeatherState::Thunderstorm: return { 0.120f, 0.70f, -1.00f };
		case EWeatherState::Clear:
		default:                          return { 0.020f, 1.00f,  0.00f };
		}
	}
}

URANDWeatherComponent::URANDWeatherComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDWeatherComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheSceneActors();

	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		Clock->OnHourPassed.AddDynamic(this, &URANDWeatherComponent::HandleHourPassed);
		Clock->OnDayPassed.AddDynamic(this, &URANDWeatherComponent::HandleDayPassed);
	}

	// Roll today's load shedding and set the opening weather look.
	SetEskomStage(FMath::RandRange(0, 8));
	ApplyWeatherVisuals();
}

void URANDWeatherComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		Clock->OnHourPassed.RemoveDynamic(this, &URANDWeatherComponent::HandleHourPassed);
		Clock->OnDayPassed.RemoveDynamic(this, &URANDWeatherComponent::HandleDayPassed);
	}

	Super::EndPlay(EndPlayReason);
}

void URANDWeatherComponent::CacheSceneActors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		Fog = *It;
		break;
	}

	// Prefer an unbound (level-wide) post-process volume.
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		PostProcess = *It;
		if (It->bUnbound)
		{
			break;
		}
	}
}

void URANDWeatherComponent::HandleHourPassed(int32 /*Day*/, int32 Hour)
{
	const bool bStormWindow = (Hour >= StormWindowStartHour && Hour < StormWindowEndHour);

	if (bStormWindow && FMath::FRand() < AfternoonStormChance)
	{
		SetWeather(EWeatherState::Thunderstorm);
	}
	else
	{
		// Outside the window, or the roll missed: the storm eases off.
		SetWeather(DecayedState(CurrentWeather));
	}
}

void URANDWeatherComponent::HandleDayPassed(int32 /*Day*/)
{
	SetEskomStage(FMath::RandRange(0, 8));
}

void URANDWeatherComponent::SetWeather(EWeatherState NewState)
{
	if (NewState == CurrentWeather)
	{
		return;
	}

	const EWeatherState OldState = CurrentWeather;
	CurrentWeather = NewState;
	ApplyWeatherVisuals();
	OnWeatherChanged.Broadcast(OldState, NewState);
}

void URANDWeatherComponent::ApplyWeatherVisuals()
{
	const FWeatherLook Look = LookFor(CurrentWeather);

	if (AExponentialHeightFog* FogActor = Fog.Get())
	{
		if (UExponentialHeightFogComponent* FogComp = FogActor->GetComponent())
		{
			FogComp->SetFogDensity(Look.FogDensity);
		}
	}

	if (APostProcessVolume* PPV = PostProcess.Get())
	{
		FPostProcessSettings& S = PPV->Settings;
		S.bOverride_ColorSaturation = true;
		S.ColorSaturation = FVector4(Look.Saturation, Look.Saturation, Look.Saturation, 1.0f);
		S.bOverride_AutoExposureBias = true;
		S.AutoExposureBias = Look.ExposureBias;
	}
}

void URANDWeatherComponent::SetEskomStage(int32 NewStage)
{
	const int32 Clamped = FMath::Clamp(NewStage, 0, 8);
	if (Clamped == EskomStage)
	{
		return;
	}

	EskomStage = Clamped;
	ApplyLoadShedding();
	OnLoadSheddingChanged.Broadcast(EskomStage);
}

void URANDWeatherComponent::ApplyLoadShedding()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const bool bLightsOff = IsLoadShedding();

	// Artificial lights only — leave the sun and sky light alone.
	for (TActorIterator<APointLight> It(World); It; ++It)
	{
		if (ULightComponent* LightComp = It->GetLightComponent())
		{
			LightComp->SetVisibility(!bLightsOff);
		}
	}
	for (TActorIterator<ASpotLight> It(World); It; ++It)
	{
		if (ULightComponent* LightComp = It->GetLightComponent())
		{
			LightComp->SetVisibility(!bLightsOff);
		}
	}
}

EWeatherState URANDWeatherComponent::DecayedState(EWeatherState State)
{
	switch (State)
	{
	case EWeatherState::Thunderstorm: return EWeatherState::HeavyRain;
	case EWeatherState::HeavyRain:    return EWeatherState::LightRain;
	case EWeatherState::LightRain:    return EWeatherState::Overcast;
	case EWeatherState::Overcast:     return EWeatherState::Clear;
	case EWeatherState::Clear:
	default:                          return EWeatherState::Clear;
	}
}
