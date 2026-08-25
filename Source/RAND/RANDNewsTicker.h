// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDNewsTicker.generated.h"

UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDNewsTicker : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDNewsTicker();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION() void HandleHourPassed(int32 Day, int32 Hour);
	void Broadcast();
};
