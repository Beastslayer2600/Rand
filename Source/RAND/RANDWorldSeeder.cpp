// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDWorldSeeder.h"
#include "RANDCharacter.h"
#include "RANDProperty.h"
#include "RANDTenderDesk.h"
#include "RANDNPCSpawner.h"
#include "RANDAmmoCrate.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

URANDWorldSeeder::URANDWorldSeeder()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDWorldSeeder::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(SeedTimer, this, &URANDWorldSeeder::Seed, 1.2f, false);
	}
}

void URANDWorldSeeder::Seed()
{
	UWorld* World = GetWorld();
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!World || !Player) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (!UGameplayStatics::GetActorOfClass(this, ARANDProperty::StaticClass()))
	{
		const FVector Loc = Player->GetActorLocation() + FVector(900.0f, -200.0f, 200.0f);
		if (ARANDProperty* Prop = World->SpawnActor<ARANDProperty>(ARANDProperty::StaticClass(), Loc, FRotator::ZeroRotator, Params))
		{
			Prop->PropertyName = TEXT("Marshalltown walk-up");
			Prop->PropertyType = ERANDPropertyType::Residential;
			Prop->PurchasePrice = 180000.0f;
			Prop->MonthlyRentalIncome = 12000.0f;
		}
	}

	if (!UGameplayStatics::GetActorOfClass(this, ARANDTenderDesk::StaticClass()))
	{
		World->SpawnActor<ARANDTenderDesk>(ARANDTenderDesk::StaticClass(),
			Player->GetActorLocation() + FVector(350.0f, 250.0f, 40.0f), FRotator::ZeroRotator, Params);
	}

	if (!UGameplayStatics::GetActorOfClass(this, ARANDAmmoCrate::StaticClass()))
	{
		World->SpawnActor<ARANDAmmoCrate>(ARANDAmmoCrate::StaticClass(),
			Player->GetActorLocation() + FVector(250.0f, -180.0f, 20.0f), FRotator::ZeroRotator, Params);
	}

	if (!UGameplayStatics::GetActorOfClass(this, ARANDNPCSpawner::StaticClass()))
	{
		if (ARANDNPCSpawner* Spawner = World->SpawnActor<ARANDNPCSpawner>(
			ARANDNPCSpawner::StaticClass(), Player->GetActorLocation(), FRotator::ZeroRotator, Params))
		{
			Spawner->District = EDistrict::MarshallTown;
			Spawner->CivilianCount = 8;
			Spawner->PoliceCount = 2;
			Spawner->CriminalCount = 1;
			Spawner->SpawnRadius = 1600.0f;
		}
	}
}
