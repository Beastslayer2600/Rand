// Copyright Fortitudo Studio. All Rights Reserved.

#include "EconomyComponent.h"

URANDEconomyComponent::URANDEconomyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDEconomyComponent::BeginPlay()
{
	Super::BeginPlay();

	Balance = StartingBalance;
}

void URANDEconomyComponent::AddFunds(float Amount, const FString& Source)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	Balance += Amount;
	RecordTransaction(Amount, Source, /*bIsCredit=*/true);
	OnBalanceChanged.Broadcast(Balance, Amount);
}

bool URANDEconomyComponent::DeductFunds(float Amount, const FString& Reason)
{
	if (Amount <= 0.0f)
	{
		return false;
	}
	if (Balance < Amount)
	{
		return false; // Insufficient funds — no-op.
	}

	Balance -= Amount;
	RecordTransaction(Amount, Reason, /*bIsCredit=*/false);
	OnBalanceChanged.Broadcast(Balance, -Amount);
	return true;
}

void URANDEconomyComponent::RecordTransaction(float Amount, const FString& Source, bool bIsCredit)
{
	FRANDTransaction& Entry = TransactionLog.AddDefaulted_GetRef();
	Entry.Amount = Amount;
	Entry.Source = Source;
	Entry.Timestamp = FDateTime::Now();
	Entry.bIsCredit = bIsCredit;
}
