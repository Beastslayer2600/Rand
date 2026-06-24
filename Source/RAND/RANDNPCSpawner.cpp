// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDNPCSpawner.h"
#include "RANDCharacter_NPC.h"

#include "Engine/World.h"

ARANDNPCSpawner::ARANDNPCSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// Default so a spawner works out of the box without assigning a subclass.
	NPCClass = ARANDCharacter_NPC::StaticClass();
}

void ARANDNPCSpawner::BeginPlay()
{
	Super::BeginPlay();
	SpawnNPCs();
}

void ARANDNPCSpawner::SpawnNPCs()
{
	UWorld* World = GetWorld();
	if (!World || !NPCClass || SpawnCount <= 0)
	{
		return;
	}

	const FVector Origin = GetActorLocation();

	FActorSpawnParameters Params;
	Params.Owner = this;
	// Spawn even if the point is blocked; UE nudges the capsule to a free spot.
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int32 i = 0; i < SpawnCount; ++i)
	{
		// Uniform scatter over the disc: sqrt(rand) keeps density even, not
		// clustered toward the centre.
		const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
		const float Dist = SpawnRadius * FMath::Sqrt(FMath::FRand());
		const FVector Location = Origin +
			FVector(Dist * FMath::Cos(Angle), Dist * FMath::Sin(Angle), 0.0f);
		const FRotator Rotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

		ARANDCharacter_NPC* NPC =
			World->SpawnActor<ARANDCharacter_NPC>(NPCClass, Location, Rotation, Params);
		if (NPC)
		{
			NPC->AssignLanguageFromDistrict(District);
		}
	}
}
