// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDWorldSeeder.h"
#include "RANDCharacter.h"
#include "RANDProperty.h"
#include "RANDTenderDesk.h"
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

	if (!UGameplayStatics::GetActorOfClass(this, ARANDProperty::StaticClass()))
	{
		const FVector Loc = Player->GetActorLocation() + FVector(900.0f, -200.0f, 200.0f);
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
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
		const FVector Loc = Player->GetActorLocation() + FVector(350.0f, 250.0f, 40.0f);
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<ARANDTenderDesk>(ARANDTenderDesk::StaticClass(), Loc, FRotator::ZeroRotator, Params);
	}
}
