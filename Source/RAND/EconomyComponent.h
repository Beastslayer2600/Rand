// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EconomyComponent.generated.h"

/**
 * FRANDTransaction — a single ledger entry in André's finances. Credits are
 * money in (tender payouts, business income); debits are money out (bribes,
 * purchases, laundering fees).
 */
USTRUCT(BlueprintType)
struct FRANDTransaction
{
	GENERATED_BODY()

	/** Always positive; bIsCredit records the direction. Currency is ZAR. */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	float Amount = 0.0f;

	/** Free-text origin/reason, e.g. "Tender: Tshwane road works" or "Bribe: SAPS captain". */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	FString Source;

	/** When the transaction was recorded. */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	FDateTime Timestamp;

	/** True for money in (credit), false for money out (debit). */
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	bool bIsCredit = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBalanceChanged, float, NewBalance, float, Delta);

/**
 * URANDEconomyComponent — André's wallet and transaction ledger.
 *
 * Tracks a single ZAR balance, exposes credit/debit operations that append to
 * a transaction log, and broadcasts OnBalanceChanged so the HUD and other
 * systems can react. The economy is the spine of the GDD's state-capture
 * progression; business income (URANDBusinessManager) flows through here.
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDEconomyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDEconomyComponent();

	// --- Configuration ------------------------------------------------------

	/** Balance André starts the game with (ZAR). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	float StartingBalance = 0.0f;

	// --- Queries ------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Economy")
	float GetBalance() const { return Balance; }

	UFUNCTION(BlueprintPure, Category = "Economy")
	bool CanAfford(float Amount) const { return Balance >= Amount; }

	/** Read-only view of the full ledger, oldest first. */
	UFUNCTION(BlueprintPure, Category = "Economy")
	const TArray<FRANDTransaction>& GetTransactionLog() const { return TransactionLog; }

	// --- Mutation -----------------------------------------------------------

	/** Add money. Amount must be positive; logs a credit and fires OnBalanceChanged. */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AddFunds(float Amount, const FString& Source);

	/**
	 * Spend money. Returns true if the funds were deducted; false (and a no-op)
	 * if the balance is insufficient. On success logs a debit and fires
	 * OnBalanceChanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool DeductFunds(float Amount, const FString& Reason);

	// --- Delegates ----------------------------------------------------------

	/** Fired on every balance change. Delta is positive for credits, negative for debits. */
	UPROPERTY(BlueprintAssignable, Category = "Economy")
	FOnBalanceChanged OnBalanceChanged;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	float Balance = 0.0f;

	UPROPERTY()
	TArray<FRANDTransaction> TransactionLog;

	/** Appends a ledger entry. */
	void RecordTransaction(float Amount, const FString& Source, bool bIsCredit);
};
