// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimeComponent.generated.h"

/**
 * Coarse phase of the in-game day, derived from the hour. Used by lighting,
 * NPC schedules, and business availability later.
 */
UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
	Dawn		UMETA(DisplayName = "Dawn"),		// 05:00–06:59
	Morning		UMETA(DisplayName = "Morning"),		// 07:00–11:59
	Afternoon	UMETA(DisplayName = "Afternoon"),	// 12:00–16:59
	Evening		UMETA(DisplayName = "Evening"),		// 17:00–20:59
	Night		UMETA(DisplayName = "Night"),		// 21:00–04:59
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMinutePassed, int32, Day, int32, Hour, int32, Minute);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHourPassed, int32, Day, int32, Hour);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayPassed, int32, Day);

/**
 * URANDTimeComponent — the authoritative in-game clock. Lives on the game mode.
 *
 * Advances Day/Hour/Minute on a real-time timer (RealSecondsPerGameMinute per
 * in-game minute) and broadcasts minute/hour/day rollovers. Other systems
 * (economy ledger, business accrual, HUD) read from it via the Get() helper
 * rather than running their own clocks.
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDTimeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDTimeComponent();

	/** Resolves the clock from the active game mode. Null if there is none. */
	static URANDTimeComponent* Get(const UObject* WorldContext);

	// --- Configuration ------------------------------------------------------

	/** Real-world seconds that represent one in-game minute. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float RealSecondsPerGameMinute = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time|Start")
	int32 StartingDay = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time|Start", meta = (ClampMin = "0", ClampMax = "23"))
	int32 StartingHour = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time|Start", meta = (ClampMin = "0", ClampMax = "59"))
	int32 StartingMinute = 0;

	// --- Queries ------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Time")
	int32 GetCurrentDay() const { return CurrentDay; }

	UFUNCTION(BlueprintPure, Category = "Time")
	int32 GetCurrentHour() const { return CurrentHour; }

	UFUNCTION(BlueprintPure, Category = "Time")
	int32 GetCurrentMinute() const { return CurrentMinute; }

	UFUNCTION(BlueprintPure, Category = "Time")
	ETimeOfDay GetTimeOfDay() const;

	/** Formatted clock for UI, e.g. "Day 1 — 14:32". */
	UFUNCTION(BlueprintPure, Category = "Time")
	FString GetTimeString() const;

	// --- Delegates ----------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnMinutePassed OnMinutePassed;

	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnHourPassed OnHourPassed;

	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnDayPassed OnDayPassed;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	int32 CurrentDay = 1;

	UPROPERTY()
	int32 CurrentHour = 8;

	UPROPERTY()
	int32 CurrentMinute = 0;

	FTimerHandle TickTimer;

	/** Advances the clock by one in-game minute and fires the rollover delegates. */
	void AdvanceMinute();
};
