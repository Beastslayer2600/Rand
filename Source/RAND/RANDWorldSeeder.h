// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDWorldSeeder.generated.h"

UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDWorldSeeder : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDWorldSeeder();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION() void Seed();
	FTimerHandle SeedTimer;
};
