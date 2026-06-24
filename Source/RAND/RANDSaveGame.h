// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EconomyComponent.h"  // FRANDTransaction
#include "BusinessManager.h"   // FRANDBusiness
#include "RANDSaveGame.generated.h"

/**
 * URANDSaveGame — the serialised snapshot of a RAND session: André's finances,
 * business portfolio, heat across the three agencies, the game clock, and his
 * last position. Written/read by URANDSaveGameManager.
 */
UCLASS()
class RAND_API URANDSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** The single save slot used by this build. */
	static const FString SlotName;

	// --- Economy ------------------------------------------------------------

	UPROPERTY()
	float Balance = 0.0f;

	/** Most recent transactions (capped to the last 50 to keep saves small). */
	UPROPERTY()
	TArray<FRANDTransaction> TransactionLog;

	// --- Businesses ---------------------------------------------------------

	UPROPERTY()
	TArray<FRANDBusiness> OwnedBusinesses;

	// --- Heat (raw 0–100 per agency) ---------------------------------------

	UPROPERTY()
	float HeatSAPS = 0.0f;

	UPROPERTY()
	float HeatHawks = 0.0f;

	UPROPERTY()
	float HeatRivals = 0.0f;

	// --- Time ---------------------------------------------------------------

	UPROPERTY()
	int32 Day = 1;

	UPROPERTY()
	int32 Hour = 8;

	UPROPERTY()
	int32 Minute = 0;

	// --- Player -------------------------------------------------------------

	UPROPERTY()
	FVector PlayerLocation = FVector::ZeroVector;
};
