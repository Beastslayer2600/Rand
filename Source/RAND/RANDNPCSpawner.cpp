// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDNPCSpawner.h"
#include "RANDCharacter_NPC.h"
#include "Engine/World.h"

namespace
{
	const TCHAR* CivilianNames[] = {
		TEXT("Lerato"), TEXT("Thabo"), TEXT("Nomsa"), TEXT("Pieter"),
		TEXT("Ayanda"), TEXT("Jabu"), TEXT("Karabo"), TEXT("Sibusiso"),
		TEXT("Refilwe"), TEXT("Andile"), TEXT("Fatima"), TEXT("Johan")
	};
	const TCHAR* PoliceNames[] = { TEXT("Constable Dlamini"), TEXT("Sgt. van Wyk"), TEXT("Constable Molefe") };
	const TCHAR* CriminalNames[] = { TEXT("Bra T"), TEXT("Shorty"), TEXT("Mavuso") };
}

ARANDNPCSpawner::ARANDNPCSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
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
	if (!World || !NPCClass) return;

	int32 Civ = CivilianCount;
	int32 Cop = PoliceCount;
	int32 Crook = CriminalCount;
	if (Civ + Cop + Crook <= 0 && SpawnCount > 0)
	{
		Civ = SpawnCount;
	}

	const FVector Origin = GetActorLocation();
	for (int32 i = 0; i < Civ; ++i) SpawnOne(Origin, 0);
	for (int32 i = 0; i < Cop; ++i) SpawnOne(Origin, 1);
	for (int32 i = 0; i < Crook; ++i) SpawnOne(Origin, 2);
}

ARANDCharacter_NPC* ARANDNPCSpawner::SpawnOne(const FVector& Origin, uint8 TypeIndex)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
	const float Dist = SpawnRadius * FMath::Sqrt(FMath::FRand());
	const FVector Location = Origin + FVector(Dist * FMath::Cos(Angle), Dist * FMath::Sin(Angle), 0.0f);
	const FRotator Rotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ARANDCharacter_NPC* NPC = World->SpawnActor<ARANDCharacter_NPC>(NPCClass, Location, Rotation, Params);
	if (!NPC) return nullptr;

	NPC->AssignLanguageFromDistrict(District);
	switch (TypeIndex)
	{
	case 1:
		NPC->NPCType = ENPCType::Police;
		NPC->NPCName = PoliceNames[FMath::RandRange(0, 2)];
		break;
	case 2:
		NPC->NPCType = ENPCType::Criminal;
		NPC->NPCName = CriminalNames[FMath::RandRange(0, 2)];
		break;
	default:
		NPC->NPCType = ENPCType::Civilian;
		NPC->NPCName = CivilianNames[FMath::RandRange(0, 11)];
		break;
	}
	return NPC;
}
