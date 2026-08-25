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
	void Reload();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AddReserve(int32 Rounds);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetArmed(bool bInArmed) { bArmed = bInArmed; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsArmed() const { return bArmed; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetMag() const { return CurrentMag; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetReserve() const { return ReserveAmmo; }

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 MagSize = 12;

private:
	bool bArmed = false;
	float LastFireTime = -1000.0f;
	int32 CurrentMag = 12;
	int32 ReserveAmmo = 24;

	void NotifyNearbyWitnesses(const FVector& ShotOrigin);
};
