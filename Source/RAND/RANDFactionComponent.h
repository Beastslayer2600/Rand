// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDFactionComponent.generated.h"

/** The organised-crime factions from GDD §5. */
UENUM(BlueprintType)
enum class ERANDFaction : uint8
{
	NigerianNetworks	UMETA(DisplayName = "Nigerian Networks"),
	CapeFlatsReach		UMETA(DisplayName = "Cape Flats Reach"),
	ZimMozNetworks		UMETA(DisplayName = "Zim/Moz Networks"),
	EasternEuropean		UMETA(DisplayName = "Eastern European"),
	TaxiIndustry		UMETA(DisplayName = "Taxi Industry"),
	ZamaZamas			UMETA(DisplayName = "Zama Zamas"),
};

/** Standing bucketed into how a faction treats André. */
UENUM(BlueprintType)
enum class ERANDFactionRelation : uint8
{
	War			UMETA(DisplayName = "War"),
	Hostile		UMETA(DisplayName = "Hostile"),
	Neutral		UMETA(DisplayName = "Neutral"),
	Partner		UMETA(DisplayName = "Partner"),
	Allied		UMETA(DisplayName = "Allied"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFactionRelationChanged,
	ERANDFaction, Faction, ERANDFactionRelation, OldRelation, ERANDFactionRelation, NewRelation);

/**
 * URANDFactionComponent — André's standing with the six crime factions, on ARANDGameMode.
 *
 * Standing runs -100..100 per faction and buckets into ERANDFactionRelation.
 * Every OfferIntervalHours a faction reaches out on the phone with a deal:
 * accepting pays (or costs) cash, draws heat and raises standing; refusing
 * lowers it. The Nigerian networks are never neutral: both answers swing hard.
 *
 * Hourly: factions at War add Rivals heat; Allied factions pay a tribute share.
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDFactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDFactionComponent();

	static URANDFactionComponent* Get(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "Factions")
	float GetStanding(ERANDFaction Faction) const;

	UFUNCTION(BlueprintCallable, Category = "Factions")
	void AddStanding(ERANDFaction Faction, float Delta);

	UFUNCTION(BlueprintPure, Category = "Factions")
	ERANDFactionRelation GetRelation(ERANDFaction Faction) const;

	UFUNCTION(BlueprintPure, Category = "Factions")
	static FString GetFactionName(ERANDFaction Faction);

	/** Number of factions currently at War with André. */
	UFUNCTION(BlueprintPure, Category = "Factions")
	int32 GetWarCount() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factions")
	int32 OfferIntervalHours = 6;

	/** Rivals heat per game hour from each faction at War. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factions")
	float WarHeatPerHour = 1.5f;

	/** ZAR per game hour from each Allied faction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factions")
	float AlliedTributePerHour = 1500.0f;

	UPROPERTY(BlueprintAssignable, Category = "Factions")
	FOnFactionRelationChanged OnRelationChanged;

	void LoadStandings(const TArray<float>& InStandings);
	void CopyStandings(TArray<float>& OutStandings) const;

protected:
	virtual void BeginPlay() override;

private:
	static constexpr int32 FactionCount = 6;
	float Standing[FactionCount] = {};

	int32 HoursSinceOffer = 0;
	int32 HoursOfferOpen = 0;
	bool bOfferOpen = false;
	ERANDFaction PendingFaction = ERANDFaction::NigerianNetworks;

	FTimerHandle BindTimer;

	UFUNCTION() void HandleHourPassed(int32 Day, int32 Hour);
	UFUNCTION() void HandlePhoneOption(FName ActionId);
	void BindPhone();
	void MakeOffer();
	void Resolve(bool bAccepted);
	void SetStandingInternal(ERANDFaction Faction, float Value);

	static ERANDFactionRelation Bucket(float Value);
	static int32 IndexOf(ERANDFaction Faction) { return static_cast<int32>(Faction); }
};
