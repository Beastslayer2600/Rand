// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDCombatComponent.generated.h"

UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDCombatComponent();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Fire();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetArmed(bool bInArmed) { bArmed = bInArmed; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsArmed() const { return bArmed; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float WeaponDamage = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float WeaponRange = 8000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float FireCooldown = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float WitnessPingRadius = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ShotHeatSAPS = 18.0f;

private:
	bool bArmed = false;
	float LastFireTime = -1000.0f;

	void NotifyNearbyWitnesses(const FVector& ShotOrigin);
};
