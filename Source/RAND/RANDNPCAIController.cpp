// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDNPCAIController.h"
#include "RANDCharacter_NPC.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"

#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

const FName ARANDNPCAIController::Key_HomeLocation     = FName(TEXT("HomeLocation"));
const FName ARANDNPCAIController::Key_PatrolRadius     = FName(TEXT("PatrolRadius"));
const FName ARANDNPCAIController::Key_PlayerNearby     = FName(TEXT("bPlayerNearby"));
const FName ARANDNPCAIController::Key_CrimeWitnessed   = FName(TEXT("bCrimeWitnessed"));
const FName ARANDNPCAIController::Key_DistanceToPlayer = FName(TEXT("DistanceToPlayer"));

ARANDNPCAIController::ARANDNPCAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.2f; // proximity checks at 5 Hz are plenty
}

void ARANDNPCAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn)
	{
		return;
	}

	HomeLocation = InPawn->GetActorLocation();
	SetupBlackboard(HomeLocation);

	if (BehaviorTreeAsset)
	{
		// Designer-authored tree drives behaviour; the blackboard is shared.
		bUsingCodeLogic = false;
		RunBehaviorTree(BehaviorTreeAsset);
	}
	else
	{
		// Built-in patrol/idle loop.
		bUsingCodeLogic = true;
		EnterIdle();
	}
}

void ARANDNPCAIController::OnUnPossess()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IdleTimer);
	}
	Super::OnUnPossess();
}

void ARANDNPCAIController::SetupBlackboard(const FVector& Home)
{
	BlackboardAsset = NewObject<UBlackboardData>(this, TEXT("RANDBlackboardData"));

	auto AddKey = [this](FName Name, TSubclassOf<UBlackboardKeyType> KeyTypeClass)
	{
		FBlackboardEntry Entry;
		Entry.EntryName = Name;
		Entry.KeyType = NewObject<UBlackboardKeyType>(BlackboardAsset, KeyTypeClass);
		BlackboardAsset->Keys.Add(Entry);
	};

	AddKey(Key_HomeLocation, UBlackboardKeyType_Vector::StaticClass());
	AddKey(Key_PatrolRadius, UBlackboardKeyType_Float::StaticClass());
	AddKey(Key_PlayerNearby, UBlackboardKeyType_Bool::StaticClass());
	AddKey(Key_CrimeWitnessed, UBlackboardKeyType_Bool::StaticClass());
	AddKey(Key_DistanceToPlayer, UBlackboardKeyType_Float::StaticClass());

	UseBlackboard(BlackboardAsset, BlackboardComp);

	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsVector(Key_HomeLocation, Home);
		BlackboardComp->SetValueAsFloat(Key_PatrolRadius, PatrolRadius);
		BlackboardComp->SetValueAsBool(Key_PlayerNearby, false);
		BlackboardComp->SetValueAsBool(Key_CrimeWitnessed, false);
		BlackboardComp->SetValueAsFloat(Key_DistanceToPlayer, BIG_NUMBER);
	}
}

void ARANDNPCAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateProximity();

	if (bUsingCodeLogic)
	{
		HandleCrimeReaction();
	}
}

void ARANDNPCAIController::UpdateProximity()
{
	APawn* Self = GetPawn();
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!Self || !PlayerChar)
	{
		return;
	}

	const float Distance = FVector::Dist(Self->GetActorLocation(), PlayerChar->GetActorLocation());
	const bool bNearby = Distance <= GreetingRadius;

	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsFloat(Key_DistanceToPlayer, Distance);
		BlackboardComp->SetValueAsBool(Key_PlayerNearby, bNearby);
	}

	if (!bUsingCodeLogic)
	{
		return;
	}

	// Greet once per approach: fire on entering range, re-arm on leaving.
	if (bNearby && !bGreetedCurrentApproach)
	{
		bGreetedCurrentApproach = true;
		if (ARANDCharacter_NPC* NPC = Cast<ARANDCharacter_NPC>(Self))
		{
			ShowLine(NPC->GetReactionLine(ESituationType::Greeting));
		}
	}
	else if (!bNearby)
	{
		bGreetedCurrentApproach = false;
	}
}

void ARANDNPCAIController::HandleCrimeReaction()
{
	if (bReactedToCrime || !BlackboardComp || !BlackboardComp->GetValueAsBool(Key_CrimeWitnessed))
	{
		return;
	}

	ARANDCharacter_NPC* NPC = Cast<ARANDCharacter_NPC>(GetPawn());
	if (!NPC)
	{
		return;
	}

	bReactedToCrime = true;
	ShowLine(NPC->GetReactionLine(ESituationType::CrimeWitnessed));

	if (NPC->NPCType == ENPCType::Police)
	{
		// Police report it — feeds heat to the player's wanted component.
		NPC->WitnessCrime(25.0f);
	}
	else if (NPC->NPCType == ENPCType::Civilian)
	{
		// Civilians run home.
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(IdleTimer);
		}
		State = EAIState::Flee;
		MoveToLocation(HomeLocation, 50.0f);
	}
}

void ARANDNPCAIController::EnterIdle()
{
	State = EAIState::Idle;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Wait = FMath::FRandRange(IdleMinSeconds, IdleMaxSeconds);
	World->GetTimerManager().SetTimer(IdleTimer, this, &ARANDNPCAIController::BeginPatrol, Wait, /*bLoop=*/false);
}

void ARANDNPCAIController::BeginPatrol()
{
	State = EAIState::Patrol;
	GoToPatrolPoint();
}

void ARANDNPCAIController::GoToPatrolPoint()
{
	UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!Nav)
	{
		EnterIdle(); // No navmesh — just keep idling.
		return;
	}

	FNavLocation Result;
	if (Nav->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, Result))
	{
		MoveToLocation(Result.Location, 50.0f);
	}
	else
	{
		EnterIdle();
	}
}

void ARANDNPCAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (!bUsingCodeLogic)
	{
		return;
	}

	// Reaching a patrol point or home returns the NPC to the idle loop.
	EnterIdle();
}

void ARANDNPCAIController::NotifyCrimeWitnessed()
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsBool(Key_CrimeWitnessed, true);
	}
}

void ARANDNPCAIController::ShowLine(const FText& Line)
{
	APawn* Self = GetPawn();
	if (!Self || Line.IsEmpty())
	{
		return;
	}

	DrawDebugString(GetWorld(), Self->GetActorLocation() + FVector(0.0f, 0.0f, 150.0f),
		Line.ToString(), nullptr, FColor::Yellow, 2.5f, true);
}
