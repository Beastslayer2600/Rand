// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDCareerComponent.generated.h"

UENUM(BlueprintType)
enum class ERANDCareerStage : uint8
{
	Consultant		UMETA(DisplayName = "Consultant"),
	Player			UMETA(DisplayName = "Player"),
	Operator		UMETA(DisplayName = "Operator"),
	Untouchable		UMETA(DisplayName = "Untouchable"),
	TheCityIsYours	UMETA(DisplayName = "The City Is Yours"),
};

UENUM(BlueprintType)
enum class ERANDEnding : uint8
{
	None				UMETA(DisplayName = "None"),
	Legitimate			UMETA(DisplayName = "Stayed Clean"),
	StateCapture		UMETA(DisplayName = "The City Is Yours"),
	Destroyed			UMETA(DisplayName = "Burned"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCareerStageChanged, ERANDCareerStage, OldStage, ERANDCareerStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCampaignEnded, ERANDEnding, Ending);

UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDCareerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDCareerComponent();

	UFUNCTION(BlueprintPure, Category = "Career")
	ERANDCareerStage GetStage() const { return Stage; }

	UFUNCTION(BlueprintPure, Category = "Career")
	ERANDEnding GetEnding() const { return Ending; }

	UFUNCTION(BlueprintPure, Category = "Career")
	bool IsCampaignComplete() const { return Ending != ERANDEnding::None; }

	UFUNCTION(BlueprintPure, Category = "Career")
	bool IsDark() const { return bIsDark; }

	UFUNCTION(BlueprintCallable, Category = "Career")
	void SetStage(ERANDCareerStage NewStage);

	UFUNCTION(BlueprintCallable, Category = "Career")
	void ResolveEnding(ERANDEnding InEnding);

	UFUNCTION(BlueprintCallable, Category = "Career")
	bool GoDark();

	UFUNCTION(BlueprintCallable, Category = "Career")
	void ComeUpForAir();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Career|Dark")
	float DarkCostZAR = 25000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Career|Dark")
	int32 DarkDurationHours = 6;

	UPROPERTY(BlueprintAssignable, Category = "Career")
	FOnCareerStageChanged OnStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Career")
	FOnCampaignEnded OnCampaignEnded;

	void LoadState(ERANDCareerStage InStage, ERANDEnding InEnding);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	ERANDCareerStage Stage = ERANDCareerStage::Consultant;

	UPROPERTY()
	ERANDEnding Ending = ERANDEnding::None;

	bool bIsDark = false;
	int32 DarkUntilDay = 0;
	int32 DarkUntilHour = 0;

	UFUNCTION()
	void HandleHourPassed(int32 Day, int32 Hour);
};
