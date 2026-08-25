// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDSARSComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInflationChanged, float, Multiplier);

UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDSARSComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDSARSComponent();

	static URANDSARSComponent* Get(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "SARS")
	float GetInflationMultiplier() const { return InflationMultiplier; }

	UFUNCTION(BlueprintPure, Category = "SARS")
	float Adjust(float AmountZAR) const { return AmountZAR * InflationMultiplier; }

	UFUNCTION(BlueprintPure, Category = "SARS")
	bool IsInvestigationActive() const { return bInvestigationActive; }

	UFUNCTION(BlueprintCallable, Category = "SARS")
	bool FileReturn();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SARS")
	float FilingThresholdZAR = 250000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SARS")
	float FilingRate = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SARS")
	float DailyInflationStep = 0.008f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SARS")
	float InvestigationHeatPerHour = 2.5f;

	UPROPERTY(BlueprintAssignable, Category = "SARS")
	FOnInflationChanged OnInflationChanged;

	void LoadState(float InMultiplier, bool bInvestigating);

protected:
	virtual void BeginPlay() override;

private:
	float InflationMultiplier = 1.0f;
	bool bInvestigationActive = false;
	bool bFiledThisWeek = false;
	int32 LastFiledDay = 0;

	UFUNCTION()
	void HandleHourPassed(int32 Day, int32 Hour);

	UFUNCTION()
	void HandleDayPassed(int32 Day);
};
