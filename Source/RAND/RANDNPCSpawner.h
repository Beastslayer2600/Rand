// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RANDLanguageTypes.h"
#include "RANDNPCSpawner.generated.h"

class ARANDCharacter_NPC;

UCLASS()
class RAND_API ARANDNPCSpawner : public AActor
{
	GENERATED_BODY()

public:
	ARANDNPCSpawner();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	EDistrict District = EDistrict::MarshallTown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TSubclassOf<ARANDCharacter_NPC> NPCClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0"))
	int32 CivilianCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0"))
	int32 PoliceCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0"))
	int32 CriminalCount = 1;

	/** Legacy single count; used only if the typed counts are all zero. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0"))
	int32 SpawnCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0.0"))
	float SpawnRadius = 1800.0f;

protected:
	virtual void BeginPlay() override;

private:
	void SpawnNPCs();
	ARANDCharacter_NPC* SpawnOne(const FVector& Origin, uint8 TypeIndex);
};
