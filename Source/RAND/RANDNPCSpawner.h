// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RANDLanguageTypes.h"
#include "RANDNPCSpawner.generated.h"

class ARANDCharacter_NPC;

/**
 * ARANDNPCSpawner — placed in a level to populate a district with NPCs.
 *
 * On BeginPlay it spawns SpawnCount NPCs of NPCClass, scattered uniformly
 * within SpawnRadius, and assigns each the district's weighted-random home
 * language via AssignLanguageFromDistrict.
 */
UCLASS()
class RAND_API ARANDNPCSpawner : public AActor
{
	GENERATED_BODY()

public:
	ARANDNPCSpawner();

	/** District whose language profile spawned NPCs draw from. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	EDistrict District = EDistrict::Hillbrow;

	/** NPC class to spawn. Defaults to ARANDCharacter_NPC. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TSubclassOf<ARANDCharacter_NPC> NPCClass;

	/** How many NPCs to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0"))
	int32 SpawnCount = 5;

	/** Radius (cm) of the disc NPCs are scattered within, centred on the spawner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0.0"))
	float SpawnRadius = 500.0f;

protected:
	virtual void BeginPlay() override;

private:
	void SpawnNPCs();
};
