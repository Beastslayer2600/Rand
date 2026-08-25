// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WantedComponent.h"
#include "RANDWantedResponseComponent.generated.h"

class ARANDCharacter_NPC;

UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDWantedResponseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDWantedResponseComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
	int32 UnitsAtWanted = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
	int32 UnitsAtManhunt = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
	int32 UnitsAtBurned = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
	float SpawnRadius = 1800.0f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TArray<TObjectPtr<ARANDCharacter_NPC>> ActiveUnits;

	FTimerHandle PulseTimer;

	UFUNCTION()
	void Pulse();

	UFUNCTION()
	void HandleHeatChanged(EAgency Agency, EHeatLevel OldLevel, EHeatLevel NewLevel);

	void BindToPlayer();
	void EnsureUnitCount(int32 Desired);
	void DespawnAll();
	ARANDCharacter_NPC* SpawnUnit(const FVector& Around);
};
