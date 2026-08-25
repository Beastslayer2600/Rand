// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EconomyComponent.h"
#include "BusinessManager.h"
#include "RANDCareerComponent.h"
#include "RANDSaveGame.generated.h"

UCLASS()
class RAND_API URANDSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static const FString SlotName;

	UPROPERTY()
	float Balance = 0.0f;

	UPROPERTY()
	TArray<FRANDTransaction> TransactionLog;

	UPROPERTY()
	TArray<FRANDBusiness> OwnedBusinesses;

	UPROPERTY()
	float HeatSAPS = 0.0f;

	UPROPERTY()
	float HeatHawks = 0.0f;

	UPROPERTY()
	float HeatRivals = 0.0f;

	UPROPERTY()
	int32 Day = 1;

	UPROPERTY()
	int32 Hour = 8;

	UPROPERTY()
	int32 Minute = 0;

	UPROPERTY()
	FVector PlayerLocation = FVector::ZeroVector;

	UPROPERTY()
	bool bAcceptedBribe = false;

	UPROPERTY()
	ERANDCareerStage CareerStage = ERANDCareerStage::Consultant;

	UPROPERTY()
	ERANDEnding Ending = ERANDEnding::None;

	UPROPERTY()
	TArray<float> ContactStandings;

	UPROPERTY()
	bool bArmed = false;

	UPROPERTY()
	float InflationMultiplier = 1.0f;

	UPROPERTY()
	bool bSARSInvestigation = false;
};
