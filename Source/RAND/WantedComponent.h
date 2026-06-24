// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WantedComponent.generated.h"

/**
 * The three law-enforcement agencies that track André's heat in RAND.
 *
 * SAPS  — South African Police Service. Street-level, first to respond.
 * Hawks — Directorate for Priority Crime Investigation. White-collar / organised crime.
 * Rivals— Rival criminal factions. Not law enforcement, but they run their own heat.
 */
UENUM(BlueprintType)
enum class EAgency : uint8
{
	SAPS	UMETA(DisplayName = "SAPS"),
	Hawks	UMETA(DisplayName = "Hawks"),
	Rivals	UMETA(DisplayName = "Rivals"),
};

/**
 * Five heat levels, identical across all agencies. The names map to the GDD
 * language; values are used for threshold comparisons in AI and event triggers.
 *
 *  0  None     — off the radar.
 *  1  Noticed  — witnesses, CCTV pings; no active pursuit yet.
 *  2  Wanted   — active patrol response dispatched.
 *  3  Manhunt  — roadblocks, helicopters; agencies coordinate.
 *  4  Burned   — shoot-on-sight; diplomatic contacts exhausted.
 */
UENUM(BlueprintType)
enum class EHeatLevel : uint8
{
	None		= 0 UMETA(DisplayName = "None"),
	Noticed		= 1 UMETA(DisplayName = "Noticed"),
	Wanted		= 2 UMETA(DisplayName = "Wanted"),
	Manhunt		= 3 UMETA(DisplayName = "Manhunt"),
	Burned		= 4 UMETA(DisplayName = "Burned"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHeatChanged,
	EAgency, Agency, EHeatLevel, OldLevel, EHeatLevel, NewLevel);

/**
 * UWantedComponent — data layer for André's criminal notoriety.
 *
 * Tracks a raw float heat value (0–100) per agency, which decays over time
 * when André stays out of sight. The float is bucketed into EHeatLevel for
 * game logic and HUD use. No AI response is wired here; AI modules subscribe
 * to OnHeatChanged when they are ready.
 *
 * Designer-facing:
 *   AddHeat(Agency, Amount)  — called by mission scripts, AI witnesses, etc.
 *   SetHeat(Agency, Amount)  — direct set for scripted events / cheats.
 *   ClearHeat(Agency)        — immediately zero out one agency.
 *   ClearAllHeat()           — full amnesty.
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API UWantedComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWantedComponent();

	// --- Configuration ------------------------------------------------------

	/** Rate at which raw heat falls per second (per agency) when decaying. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat|Decay")
	float HeatDecayRate = 5.0f;

	/**
	 * Seconds the player must remain out of sight before decay begins.
	 * Resets each time AddHeat is called.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat|Decay")
	float DecayCooldown = 10.0f;

	// --- Mutation -----------------------------------------------------------

	/** Increase raw heat for an agency (clamped 0–100). Resets the decay timer. */
	UFUNCTION(BlueprintCallable, Category = "Heat")
	void AddHeat(EAgency Agency, float Amount);

	/** Directly set raw heat for an agency (clamped 0–100). */
	UFUNCTION(BlueprintCallable, Category = "Heat")
	void SetHeat(EAgency Agency, float Amount);

	/** Zero out a single agency's heat immediately. */
	UFUNCTION(BlueprintCallable, Category = "Heat")
	void ClearHeat(EAgency Agency);

	/** Zero out all agencies' heat immediately. */
	UFUNCTION(BlueprintCallable, Category = "Heat")
	void ClearAllHeat();

	// --- Queries ------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Heat")
	float GetRawHeat(EAgency Agency) const;

	UFUNCTION(BlueprintPure, Category = "Heat")
	EHeatLevel GetHeatLevel(EAgency Agency) const;

	/** Highest heat level across all agencies — useful for global response logic. */
	UFUNCTION(BlueprintPure, Category = "Heat")
	EHeatLevel GetPeakHeatLevel() const;

	// --- Delegates ----------------------------------------------------------

	/** Fired when an agency's bucketed EHeatLevel changes (not on every raw float tick). */
	UPROPERTY(BlueprintAssignable, Category = "Heat")
	FOnHeatChanged OnHeatChanged;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	static constexpr int32 AgencyCount = 3;

	float RawHeat[AgencyCount]     = {};
	float DecayTimer[AgencyCount]  = {};

	/** Converts a raw 0–100 float to the appropriate EHeatLevel bucket. */
	static EHeatLevel BucketHeat(float Raw);

	/**
	 * Applies a raw heat change to one agency, fires OnHeatChanged if the
	 * bucketed level transitions, and resets the decay cooldown when heat rises.
	 */
	void ApplyHeat(EAgency Agency, float NewRaw, bool bResetCooldown);

	static int32 AgencyIndex(EAgency Agency) { return static_cast<int32>(Agency); }
};
