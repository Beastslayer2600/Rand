// Copyright Fortitudo Studio. All Rights Reserved.

#include "TimeComponent.h"

#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"

URANDTimeComponent::URANDTimeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

URANDTimeComponent* URANDTimeComponent::Get(const UObject* WorldContext)
{
	if (!WorldContext)
	{
		return nullptr;
	}
	const UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		return nullptr;
	}
	if (AGameModeBase* GameMode = World->GetAuthGameMode())
	{
		return GameMode->FindComponentByClass<URANDTimeComponent>();
	}
	return nullptr;
}

void URANDTimeComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentDay = FMath::Max(1, StartingDay);
	CurrentHour = FMath::Clamp(StartingHour, 0, 23);
	CurrentMinute = FMath::Clamp(StartingMinute, 0, 59);

	if (UWorld* World = GetWorld())
	{
		const float Interval = FMath::Max(0.01f, RealSecondsPerGameMinute);
		World->GetTimerManager().SetTimer(TickTimer, this,
			&URANDTimeComponent::AdvanceMinute, Interval, /*bLoop=*/true);
	}
}

void URANDTimeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void URANDTimeComponent::AdvanceMinute()
{
	bool bHourRolled = false;
	bool bDayRolled = false;

	if (++CurrentMinute >= 60)
	{
		CurrentMinute = 0;
		bHourRolled = true;

		if (++CurrentHour >= 24)
		{
			CurrentHour = 0;
			++CurrentDay;
			bDayRolled = true;
		}
	}

	// Minute always; then the coarser rollovers, ascending.
	OnMinutePassed.Broadcast(CurrentDay, CurrentHour, CurrentMinute);
	if (bHourRolled)
	{
		OnHourPassed.Broadcast(CurrentDay, CurrentHour);
	}
	if (bDayRolled)
	{
		OnDayPassed.Broadcast(CurrentDay);
	}
}

void URANDTimeComponent::SetPaused(bool bNewPaused)
{
	if (bPaused == bNewPaused)
	{
		return;
	}
	bPaused = bNewPaused;

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		if (bPaused)
		{
			TimerManager.PauseTimer(TickTimer);
		}
		else
		{
			TimerManager.UnPauseTimer(TickTimer);
		}
	}
}

ETimeOfDay URANDTimeComponent::GetTimeOfDay() const
{
	if (CurrentHour >= 5 && CurrentHour <= 6)   return ETimeOfDay::Dawn;
	if (CurrentHour >= 7 && CurrentHour <= 11)  return ETimeOfDay::Morning;
	if (CurrentHour >= 12 && CurrentHour <= 16) return ETimeOfDay::Afternoon;
	if (CurrentHour >= 17 && CurrentHour <= 20) return ETimeOfDay::Evening;
	return ETimeOfDay::Night; // 21:00–04:59
}

FString URANDTimeComponent::GetTimeString() const
{
	// 0x2014 is the em dash, emitted via %c so the source stays pure ASCII and
	// no source-file encoding can mangle it.
	return FString::Printf(TEXT("Day %d %c %02d:%02d"),
		CurrentDay, static_cast<TCHAR>(0x2014), CurrentHour, CurrentMinute);
}
