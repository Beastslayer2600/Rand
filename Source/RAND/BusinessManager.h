// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WantedComponent.h"
#include "BusinessManager.generated.h"

class URANDEconomyComponent;
class URANDTimeComponent;

/**
 * Where a business sits on the legality spectrum — drives heat and income.
 *
 * Legitimate — clean fronts (logistics, security firms). No heat.
 * Grey       — exploiting the system but defensible (inflated tenders). Mild heat.
 * Black      — outright criminal (smuggling, protection). Generates Hawks heat.
 */
UENUM(BlueprintType)
enum class ERANDBusinessType : uint8
{
	Legitimate	UMETA(DisplayName = "Legitimate"),
	Grey		UMETA(DisplayName = "Grey"),
	Black		UMETA(DisplayName = "Black"),
};

/**
 * FRANDBusiness — one revenue-generating enterprise in André's portfolio.
 * Passive income and heat are expressed as per-hour rates and accrued in
 * per-game-minute slices by the business manager.
 */
USTRUCT(BlueprintType)
struct FRANDBusiness
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Business")
	FString BusinessName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Business")
	ERANDBusinessType BusinessType = ERANDBusinessType::Legitimate;

	/** Gross passive income produced per in-game hour (ZAR). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Business")
	float PassiveIncomePerHour = 0.0f;

	/** Heat produced per in-game hour (only black-tier businesses generate heat). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Business")
	float HeatGenerationRate = 0.0f;

	/** Inactive businesses neither earn nor generate heat (raided, shut down, paused). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Business")
	bool bIsActive = true;
};

/**
 * URANDBusinessManager — André's portfolio of enterprises.
 *
 * Holds the owned businesses and, on a game-minute cadence, accrues passive
 * income into the economy component and feeds heat from active black-tier
 * businesses into the wanted component. All three components live on
 * ARANDCharacter; the manager finds them on its owner at BeginPlay.
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDBusinessManager : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDBusinessManager();

	// --- Configuration ------------------------------------------------------

	/**
	 * Fallback cadence (real seconds per in-game minute) used only when no
	 * URANDTimeComponent clock is present. When the game clock exists, accrual
	 * is driven by its OnMinutePassed and this value is ignored.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Business|Time")
	float RealSecondsPerGameMinute = 1.0f;

	/** Agency that black-tier income attracts (priority/organised crime → Hawks). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Business|Heat")
	EAgency BlackTierHeatAgency = EAgency::Hawks;

	// --- Portfolio management ----------------------------------------------

	/** Add a business to the portfolio. */
	UFUNCTION(BlueprintCallable, Category = "Business")
	void AddBusiness(const FRANDBusiness& Business);

	/** Remove the first business whose name matches. Returns true if one was removed. */
	UFUNCTION(BlueprintCallable, Category = "Business")
	bool RemoveBusiness(const FString& BusinessName);

	/** Read-only view of the portfolio. */
	UFUNCTION(BlueprintPure, Category = "Business")
	const TArray<FRANDBusiness>& GetBusinesses() const { return Businesses; }

	/** Sum of PassiveIncomePerHour across active businesses (ZAR/hour). */
	UFUNCTION(BlueprintPure, Category = "Business")
	float GetTotalPassiveIncomePerHour() const;

	/**
	 * Accrues one game-minute of income and heat. Called automatically by the
	 * timer; exposed so missions/tests can advance it deterministically.
	 */
	UFUNCTION(BlueprintCallable, Category = "Business")
	void TickPassiveIncome();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TArray<FRANDBusiness> Businesses;

	TWeakObjectPtr<URANDEconomyComponent> Economy;
	TWeakObjectPtr<UWantedComponent> Wanted;
	TWeakObjectPtr<URANDTimeComponent> Time;

	/** Fallback timer, used only when there is no game clock to subscribe to. */
	FTimerHandle AccrualTimer;

	/** Bound to the clock's OnMinutePassed; forwards to TickPassiveIncome. */
	UFUNCTION()
	void HandleGameMinute(int32 Day, int32 Hour, int32 Minute);
};
