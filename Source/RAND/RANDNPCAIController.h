// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RANDNPCAIController.generated.h"

class UBlackboardComponent;
class UBlackboardData;
class UBehaviorTree;

/**
 * ARANDNPCAIController — ambient behaviour for ARANDCharacter_NPC.
 *
 * Sets up a Blackboard (built at runtime) with the keys HomeLocation,
 * PatrolRadius, bPlayerNearby, bCrimeWitnessed and DistanceToPlayer. If a
 * BehaviorTreeAsset is assigned by a designer it runs that; otherwise it drives
 * the equivalent logic in C++ so NPCs behave with no editor assets:
 *
 *   Patrol (random reachable point within PatrolRadius of home) -> Idle (3-5s)
 *   -> repeat. Player within GreetingRadius: play a greeting line once. Crime
 *   witnessed: play a reaction line, then Civilians flee home and Police report.
 *
 * Patrol needs a NavMesh (NavMeshBoundsVolume) in the level.
 */
UCLASS()
class RAND_API ARANDNPCAIController : public AAIController
{
	GENERATED_BODY()

public:
	ARANDNPCAIController();

	/** Optional designer Behaviour Tree; when set it runs instead of the C++ logic. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolRadius = 1200.0f;

	/** Distance (cm) at which the NPC greets the player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float GreetingRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Idle")
	float IdleMinSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Idle")
	float IdleMaxSeconds = 5.0f;

	/** External hook: flag that this NPC has witnessed a crime. */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void NotifyCrimeWitnessed();

	// Blackboard key names (public so a designer BT can match them).
	static const FName Key_HomeLocation;
	static const FName Key_PatrolRadius;
	static const FName Key_PlayerNearby;
	static const FName Key_CrimeWitnessed;
	static const FName Key_DistanceToPlayer;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
	UPROPERTY()
	TObjectPtr<UBlackboardComponent> BlackboardComp;

	UPROPERTY()
	TObjectPtr<UBlackboardData> BlackboardAsset;

	enum class EAIState : uint8 { Idle, Patrol, Flee };
	EAIState State = EAIState::Idle;

	/** True when running the built-in C++ behaviour (no BT asset assigned). */
	bool bUsingCodeLogic = false;
	bool bGreetedCurrentApproach = false;
	bool bReactedToCrime = false;

	FVector HomeLocation = FVector::ZeroVector;
	FTimerHandle IdleTimer;

	void SetupBlackboard(const FVector& Home);
	void EnterIdle();
	void BeginPatrol();
	void GoToPatrolPoint();

	void UpdateProximity();
	void HandleCrimeReaction();

	/** Surfaces a spoken reaction line above the NPC's head. */
	void ShowLine(const FText& Line);
};
