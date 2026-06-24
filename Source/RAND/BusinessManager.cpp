// Copyright Fortitudo Studio. All Rights Reserved.

#include "BusinessManager.h"
#include "EconomyComponent.h"

#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Engine/World.h"

URANDBusinessManager::URANDBusinessManager()
{
	// Income/heat accrual is timer-driven, not per-frame.
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDBusinessManager::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		Economy = Owner->FindComponentByClass<URANDEconomyComponent>();
		Wanted = Owner->FindComponentByClass<UWantedComponent>();
	}

	// Drive accrual on a fixed real-time cadence, each tick = one game-minute.
	if (UWorld* World = GetWorld())
	{
		const float Interval = FMath::Max(0.01f, RealSecondsPerGameMinute);
		World->GetTimerManager().SetTimer(AccrualTimer, this,
			&URANDBusinessManager::TickPassiveIncome, Interval, /*bLoop=*/true);
	}
}

void URANDBusinessManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AccrualTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void URANDBusinessManager::AddBusiness(const FRANDBusiness& Business)
{
	Businesses.Add(Business);
}

bool URANDBusinessManager::RemoveBusiness(const FString& BusinessName)
{
	const int32 Index = Businesses.IndexOfByPredicate(
		[&BusinessName](const FRANDBusiness& B) { return B.BusinessName == BusinessName; });

	if (Index != INDEX_NONE)
	{
		Businesses.RemoveAt(Index);
		return true;
	}
	return false;
}

float URANDBusinessManager::GetTotalPassiveIncomePerHour() const
{
	float Total = 0.0f;
	for (const FRANDBusiness& B : Businesses)
	{
		if (B.bIsActive)
		{
			Total += B.PassiveIncomePerHour;
		}
	}
	return Total;
}

void URANDBusinessManager::TickPassiveIncome()
{
	// One call represents a single in-game minute: take 1/60th of the hourly rates.
	constexpr float MinuteFraction = 1.0f / 60.0f;

	float IncomeThisMinute = 0.0f;
	float BlackHeatThisMinute = 0.0f;

	for (const FRANDBusiness& B : Businesses)
	{
		if (!B.bIsActive)
		{
			continue;
		}

		IncomeThisMinute += B.PassiveIncomePerHour * MinuteFraction;

		if (B.BusinessType == ERANDBusinessType::Black)
		{
			BlackHeatThisMinute += B.HeatGenerationRate * MinuteFraction;
		}
	}

	if (IncomeThisMinute > 0.0f)
	{
		if (URANDEconomyComponent* Econ = Economy.Get())
		{
			Econ->AddFunds(IncomeThisMinute, TEXT("Business: passive income"));
		}
	}

	if (BlackHeatThisMinute > 0.0f)
	{
		if (UWantedComponent* Heat = Wanted.Get())
		{
			Heat->AddHeat(BlackTierHeatAgency, BlackHeatThisMinute);
		}
	}
}
