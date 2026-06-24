// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "RANDLanguageTypes.h"
#include "RANDDistrictVolume.generated.h"

/**
 * ARANDDistrictVolume — a level volume tagging a region as a specific EDistrict.
 *
 * At BeginPlay it finds every ARANDCharacter_NPC whose location falls inside the
 * volume and assigns it that district's weighted-random home language. This is
 * how hand-placed NPCs pick up their neighbourhood's language mix; spawner-
 * created NPCs are assigned directly by the spawner instead.
 */
UCLASS()
class RAND_API ARANDDistrictVolume : public AVolume
{
	GENERATED_BODY()

public:
	ARANDDistrictVolume();

	/** Which district this volume represents. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "District")
	EDistrict District = EDistrict::Hillbrow;

protected:
	virtual void BeginPlay() override;
};
