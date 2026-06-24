// Copyright Fortitudo Studio. All Rights Reserved.

#include "WantedComponent.h"

UWantedComponent::UWantedComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.25f; // Decay runs at 4 Hz — no need for every frame.
}

void UWantedComponent::BeginPlay()
{
	Super::BeginPlay();

	for (int32 i = 0; i < AgencyCount; ++i)
	{
		RawHeat[i]    = 0.0f;
		DecayTimer[i] = 0.0f;
	}
}

void UWantedComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (int32 i = 0; i < AgencyCount; ++i)
	{
		if (RawHeat[i] <= 0.0f)
		{
			continue;
		}

		if (DecayTimer[i] > 0.0f)
		{
			DecayTimer[i] -= DeltaTime;
			continue;
		}

		const float Decay = HeatDecayRate * DeltaTime;
		const float NewRaw = FMath::Max(0.0f, RawHeat[i] - Decay);
		ApplyHeat(static_cast<EAgency>(i), NewRaw, /*bResetCooldown=*/false);
	}
}

// --- Mutation ---------------------------------------------------------------

void UWantedComponent::AddHeat(EAgency Agency, float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}
	const int32 Idx = AgencyIndex(Agency);
	const float NewRaw = FMath::Clamp(RawHeat[Idx] + Amount, 0.0f, 100.0f);
	ApplyHeat(Agency, NewRaw, /*bResetCooldown=*/true);
}

void UWantedComponent::SetHeat(EAgency Agency, float Amount)
{
	const float NewRaw = FMath::Clamp(Amount, 0.0f, 100.0f);
	const bool bRising = NewRaw > RawHeat[AgencyIndex(Agency)];
	ApplyHeat(Agency, NewRaw, bRising);
}

void UWantedComponent::ClearHeat(EAgency Agency)
{
	ApplyHeat(Agency, 0.0f, /*bResetCooldown=*/false);
	DecayTimer[AgencyIndex(Agency)] = 0.0f;
}

void UWantedComponent::ClearAllHeat()
{
	for (int32 i = 0; i < AgencyCount; ++i)
	{
		ClearHeat(static_cast<EAgency>(i));
	}
}

// --- Queries ----------------------------------------------------------------

float UWantedComponent::GetRawHeat(EAgency Agency) const
{
	return RawHeat[AgencyIndex(Agency)];
}

EHeatLevel UWantedComponent::GetHeatLevel(EAgency Agency) const
{
	return BucketHeat(RawHeat[AgencyIndex(Agency)]);
}

EHeatLevel UWantedComponent::GetPeakHeatLevel() const
{
	EHeatLevel Peak = EHeatLevel::None;
	for (int32 i = 0; i < AgencyCount; ++i)
	{
		const EHeatLevel Level = BucketHeat(RawHeat[i]);
		if (static_cast<uint8>(Level) > static_cast<uint8>(Peak))
		{
			Peak = Level;
		}
	}
	return Peak;
}

// --- Private ----------------------------------------------------------------

EHeatLevel UWantedComponent::BucketHeat(float Raw)
{
	// Thresholds: 0, 1–24, 25–49, 50–74, 75–100
	if (Raw <= 0.0f)  return EHeatLevel::None;
	if (Raw <  25.0f) return EHeatLevel::Noticed;
	if (Raw <  50.0f) return EHeatLevel::Wanted;
	if (Raw <  75.0f) return EHeatLevel::Manhunt;
	return EHeatLevel::Burned;
}

void UWantedComponent::ApplyHeat(EAgency Agency, float NewRaw, bool bResetCooldown)
{
	const int32 Idx = AgencyIndex(Agency);
	const EHeatLevel OldLevel = BucketHeat(RawHeat[Idx]);
	const EHeatLevel NewLevel = BucketHeat(NewRaw);

	RawHeat[Idx] = NewRaw;

	if (bResetCooldown)
	{
		DecayTimer[Idx] = DecayCooldown;
	}

	if (OldLevel != NewLevel)
	{
		OnHeatChanged.Broadcast(Agency, OldLevel, NewLevel);
	}
}
