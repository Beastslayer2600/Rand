// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDTenderBoard.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTenderAwarded, const FString&, TenderName, float, ContractValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlacklisted, int32, Days);

/**
 * URANDTenderBoard — the open procurement market (GDD §4 Tender System), on ARANDGameMode.
 *
 * Each morning a municipal tender lands on the phone. André can:
 *   - Bid clean: needs a CIDB grade that covers the contract value; the win
 *     chance comes from the BEE preference points (80/20 system).
 *   - Pay the SCM official: a cut of the contract value buys the award
 *     outright, over-grade or not, but draws Hawks heat and whistle-blower risk.
 *   - Pass.
 *
 * Won contracts pay a margin and, every ContractsPerGrade wins, lift the CIDB
 * grade. Whistle-blower risk is rolled daily; exposure means Hawks heat and a
 * National Treasury blacklist. A registered BEE front (fronting) lifts the
 * BEE level to 1 and lets André keep bidding while blacklisted, at the cost of
 * extra whistle-blower risk on every win.
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDTenderBoard : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDTenderBoard();

	static URANDTenderBoard* Get(const UObject* WorldContext);

	// --- Queries ------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Tender") int32 GetCIDBGrade() const { return CIDBGrade; }
	/** Effective B-BBEE level for scoring: a registered front reads as level 1. */
	UFUNCTION(BlueprintPure, Category = "Tender") int32 GetBEELevel() const { return bHasFront ? 1 : BEELevel; }
	/** VCP's own B-BBEE level, ignoring any front. */
	UFUNCTION(BlueprintPure, Category = "Tender") int32 GetBaseBEELevel() const { return BEELevel; }
	UFUNCTION(BlueprintPure, Category = "Tender") int32 GetContractsWon() const { return ContractsWon; }
	UFUNCTION(BlueprintPure, Category = "Tender") bool HasFront() const { return bHasFront; }
	UFUNCTION(BlueprintPure, Category = "Tender") bool IsBlacklisted() const { return BlacklistDays > 0; }
	UFUNCTION(BlueprintPure, Category = "Tender") int32 GetBlacklistDays() const { return BlacklistDays; }
	UFUNCTION(BlueprintPure, Category = "Tender") float GetWhistleblowerRisk() const { return WhistleblowerRisk; }

	/** Highest contract value (ZAR) a CIDB grade may bid on. */
	UFUNCTION(BlueprintPure, Category = "Tender") static float GradeLimit(int32 Grade);

	/** 80/20 preference points for a B-BBEE level (1 best .. 8; 0 = non-compliant). */
	UFUNCTION(BlueprintPure, Category = "Tender") static int32 PreferencePoints(int32 Level);

	// --- Configuration ------------------------------------------------------

	/** Hour of day the morning tender is posted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tender", meta = (ClampMin = "0", ClampMax = "23"))
	int32 PostingHour = 9;

	/** Share of contract value André keeps as margin on a win. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tender")
	float WinMargin = 0.10f;

	/** Share of contract value the SCM official wants to rig the award. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tender")
	float BribeRate = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tender")
	int32 ContractsPerGrade = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tender")
	float FrontCost = 150000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tender")
	int32 BlacklistLengthDays = 14;

	// --- Delegates ----------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Tender") FOnTenderAwarded OnTenderAwarded;
	UPROPERTY(BlueprintAssignable, Category = "Tender") FOnBlacklisted OnBlacklisted;

	// --- Save ---------------------------------------------------------------

	void LoadState(int32 InGrade, int32 InBEELevel, int32 InContractsWon, bool bInHasFront,
		int32 InBlacklistDays, float InWhistleblowerRisk);

protected:
	virtual void BeginPlay() override;

private:
	int32 CIDBGrade = 2;
	int32 BEELevel = 6;
	int32 ContractsWon = 0;
	bool bHasFront = false;
	int32 BlacklistDays = 0;
	float WhistleblowerRisk = 0.0f;

	// Today's tender.
	bool bTenderOpen = false;
	bool bPostedToday = false;
	int32 HoursTenderOpen = 0;
	FString TenderName;
	float TenderValue = 0.0f;

	// Fronting offer.
	bool bFrontOfferOpen = false;
	bool bFrontOffered = false;
	bool bFrontRetryTomorrow = false;
	int32 HoursFrontOfferOpen = 0;

	FTimerHandle BindTimer;

	UFUNCTION() void HandleHourPassed(int32 Day, int32 Hour);
	UFUNCTION() void HandleDayPassed(int32 Day);
	UFUNCTION() void HandlePhoneOption(FName ActionId);
	void BindPhone();

	void PostTender();
	void BidClean();
	void RigTender();
	void Award();
	void OfferFront();
	void Expose();
	void Send(const FString& Sender, const FString& Text);
};
