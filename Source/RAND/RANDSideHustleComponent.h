// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDSideHustleComponent.generated.h"

UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDSideHustleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDSideHustleComponent();

	static URANDSideHustleComponent* Get(const UObject* WorldContext);

protected:
	virtual void BeginPlay() override;

private:
	bool bOfferOpen = false;
	int32 HoursSinceOffer = 0;
	FName PendingAccept = NAME_None;

	UFUNCTION() void HandleHourPassed(int32 Day, int32 Hour);
	UFUNCTION() void HandlePhoneOption(FName ActionId);
	void BindPhone();
	void OfferJob();
	void Resolve(FName ActionId);

	FTimerHandle BindTimer;
};
