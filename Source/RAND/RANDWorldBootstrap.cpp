// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDWorldBootstrap.h"
#include "RANDCharacter.h"
#include "RANDVehicle.h"
#include "RANDProperty.h"
#include "RANDTenderDesk.h"
#include "RANDCharacter_NPC.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

URANDWorldBootstrap::URANDWorldBootstrap()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDWorldBootstrap::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(SeedTimer, this, &URANDWorldBootstrap::SeedWorld, 1.4f, false);
	}
}

void URANDWorldBootstrap::SeedWorld()
{
	UWorld* World = GetWorld();
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!World || !Player) return;

	const FVector Origin = Player->GetActorLocation();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (!UGameplayStatics::GetActorOfClass(this, ARANDVehicle::StaticClass()))
	{
		const FVector CarLoc = Origin + FVector(600.0f, 400.0f, 40.0f);
		if (ARANDVehicle* Car = World->SpawnActor<ARANDVehicle>(ARANDVehicle::StaticClass(), CarLoc, FRotator::ZeroRotator, Params))
		{
			Car->bOwned = false;
		}
	}

	if (!UGameplayStatics::GetActorOfClass(this, ARANDProperty::StaticClass()))
	{
		const FVector FlatLoc = Origin + FVector(-700.0f, 200.0f, 200.0f);
		if (ARANDProperty* Flat = World->SpawnActor<ARANDProperty>(ARANDProperty::StaticClass(), FlatLoc, FRotator::ZeroRotator, Params))
		{
			Flat->PropertyName = TEXT("Hillbrow walk-up");
			Flat->PropertyType = ERANDPropertyType::Residential;
			Flat->PurchasePrice = 180000.0f;
			Flat->MonthlyRentalIncome = 12000.0f;
		}
	}

	if (!UGameplayStatics::GetActorOfClass(this, ARANDTenderDesk::StaticClass()))
	{
		const FVector DeskLoc = Origin + FVector(900.0f, -200.0f, 40.0f);
		World->SpawnActor<ARANDTenderDesk>(ARANDTenderDesk::StaticClass(), DeskLoc, FRotator::ZeroRotator, Params);
	}

	TArray<AActor*> ExistingNPCs;
	UGameplayStatics::GetAllActorsOfClass(World, ARANDCharacter_NPC::StaticClass(), ExistingNPCs);
	if (ExistingNPCs.Num() < 3)
	{
		const TCHAR* Names[] = { TEXT("Street vendor"), TEXT("Metro cop"), TEXT("Queue auntie") };
		const ENPCType Types[] = { ENPCType::Civilian, ENPCType::Police, ENPCType::Civilian };
		for (int32 i = 0; i < 3; ++i)
		{
			const FVector Loc = Origin + FVector(300.0f + i * 220.0f, -500.0f, 0.0f);
			if (ARANDCharacter_NPC* NPC = World->SpawnActor<ARANDCharacter_NPC>(ARANDCharacter_NPC::StaticClass(), Loc, FRotator::ZeroRotator, Params))
			{
				NPC->NPCName = Names[i];
				NPC->NPCType = Types[i];
			}
		}
	}
}
