// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDWorldBootstrap.generated.h"

/** Drops a car, a Hillbrow cube-flat and a tender desk into L_TestBox so PIE is not an empty slab. */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDWorldBootstrap : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDWorldBootstrap();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION() void SeedWorld();
	FTimerHandle SeedTimer;
};
