// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDWantedResponseComponent.h"
#include "RANDCharacter.h"
#include "RANDCharacter_NPC.h"
#include "RANDNPCAIController.h"
#include "WantedComponent.h"
#include "RANDLanguageTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NavigationSystem.h"

URANDWantedResponseComponent::URANDWantedResponseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDWantedResponseComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(PulseTimer, this,
			&URANDWantedResponseComponent::Pulse, 2.0f, true, 1.5f);
	}
}

void URANDWantedResponseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DespawnAll();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PulseTimer);
	}
	Super::EndPlay(EndPlayReason);
}

void URANDWantedResponseComponent::BindToPlayer()
{
	if (ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UWantedComponent* Wanted = Player->GetWantedComponent())
		{
			if (!Wanted->OnHeatChanged.IsAlreadyBound(this, &URANDWantedResponseComponent::HandleHeatChanged))
			{
				Wanted->OnHeatChanged.AddDynamic(this, &URANDWantedResponseComponent::HandleHeatChanged);
			}
		}
	}
}

void URANDWantedResponseComponent::Pulse()
{
	BindToPlayer();
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player || !Player->GetWantedComponent())
	{
		return;
	}

	const EHeatLevel Peak = Player->GetWantedComponent()->GetPeakHeatLevel();
	int32 Desired = 0;
	switch (Peak)
	{
	case EHeatLevel::Wanted:  Desired = UnitsAtWanted;  break;
	case EHeatLevel::Manhunt: Desired = UnitsAtManhunt; break;
	case EHeatLevel::Burned:  Desired = UnitsAtBurned;  break;
	default: break;
	}
	EnsureUnitCount(Desired);
}

void URANDWantedResponseComponent::HandleHeatChanged(EAgency, EHeatLevel, EHeatLevel)
{
	Pulse();
}

void URANDWantedResponseComponent::EnsureUnitCount(int32 Desired)
{
	ActiveUnits.RemoveAll([](const TObjectPtr<ARANDCharacter_NPC>& Unit) { return !IsValid(Unit); });
	while (ActiveUnits.Num() > Desired)
	{
		ARANDCharacter_NPC* Extra = ActiveUnits.Pop();
		if (IsValid(Extra)) Extra->Destroy();
	}

	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player) return;

	while (ActiveUnits.Num() < Desired)
	{
		if (ARANDCharacter_NPC* Unit = SpawnUnit(Player->GetActorLocation()))
		{
			ActiveUnits.Add(Unit);
		}
		else break;
	}
}

ARANDCharacter_NPC* URANDWantedResponseComponent::SpawnUnit(const FVector& Around)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FVector SpawnLoc = Around + FVector(SpawnRadius, 0.0f, 0.0f);
	if (UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(World))
	{
		FNavLocation Result;
		if (Nav->GetRandomReachablePointInRadius(Around, SpawnRadius, Result))
		{
			SpawnLoc = Result.Location;
		}
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ARANDCharacter_NPC* Unit = World->SpawnActor<ARANDCharacter_NPC>(ARANDCharacter_NPC::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
	if (!Unit) return nullptr;

	Unit->NPCName = TEXT("SAPS Constable");
	Unit->NPCType = ENPCType::Police;
	Unit->LanguageGroup = ELanguageGroup::English;
	if (ARANDNPCAIController* AI = Cast<ARANDNPCAIController>(Unit->GetController()))
	{
		AI->NotifyCrimeWitnessed();
	}
	return Unit;
}

void URANDWantedResponseComponent::DespawnAll()
{
	for (ARANDCharacter_NPC* Unit : ActiveUnits)
	{
		if (IsValid(Unit)) Unit->Destroy();
	}
	ActiveUnits.Reset();
}
