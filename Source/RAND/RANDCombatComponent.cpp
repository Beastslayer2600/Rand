// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDCombatComponent.h"
#include "RANDCharacter.h"
#include "RANDCharacter_NPC.h"
#include "RANDNPCAIController.h"
#include "WantedComponent.h"
#include "HealthComponent.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

URANDCombatComponent::URANDCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDCombatComponent::Fire()
{
	if (!bArmed)
	{
		return;
	}

	UWorld* World = GetWorld();
	ARANDCharacter* Shooter = Cast<ARANDCharacter>(GetOwner());
	if (!World || !Shooter)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	if (Now - LastFireTime < FireCooldown)
	{
		return;
	}
	LastFireTime = Now;

	FVector Start = Shooter->GetActorLocation() + FVector(0.0f, 0.0f, 70.0f);
	FVector Dir = Shooter->GetActorForwardVector();
	if (APlayerController* PC = Cast<APlayerController>(Shooter->GetController()))
	{
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);
		Start = CamLoc;
		Dir = CamRot.Vector();
	}

	const FVector End = Start + Dir * WeaponRange;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(RANDFire), false, Shooter);
	const bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params);

	DrawDebugLine(World, Start, bHit ? Hit.ImpactPoint : End, FColor::Orange, false, 0.15f, 0, 1.5f);

	if (bHit && Hit.GetActor())
	{
		UGameplayStatics::ApplyPointDamage(
			Hit.GetActor(), WeaponDamage, Dir, Hit,
			Shooter->GetController(), Shooter, UDamageType::StaticClass());

		if (ARANDCharacter_NPC* NPC = Cast<ARANDCharacter_NPC>(Hit.GetActor()))
		{
			if (ARANDNPCAIController* AI = Cast<ARANDNPCAIController>(NPC->GetController()))
			{
				AI->NotifyCrimeWitnessed();
			}
		}
	}

	if (UWantedComponent* Wanted = Shooter->GetWantedComponent())
	{
		Wanted->AddHeat(EAgency::SAPS, ShotHeatSAPS);
	}

	NotifyNearbyWitnesses(Start);
}

void URANDCombatComponent::NotifyNearbyWitnesses(const FVector& ShotOrigin)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<AActor*> NPCs;
	UGameplayStatics::GetAllActorsOfClass(World, ARANDCharacter_NPC::StaticClass(), NPCs);
	for (AActor* Actor : NPCs)
	{
		if (!Actor || FVector::Dist(Actor->GetActorLocation(), ShotOrigin) > WitnessPingRadius)
		{
			continue;
		}
		if (ARANDCharacter_NPC* NPC = Cast<ARANDCharacter_NPC>(Actor))
		{
			if (ARANDNPCAIController* AI = Cast<ARANDNPCAIController>(NPC->GetController()))
			{
				AI->NotifyCrimeWitnessed();
			}
			NPC->WitnessCrime(8.0f);
		}
	}
}
