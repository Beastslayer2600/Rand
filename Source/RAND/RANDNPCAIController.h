// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RANDNPCAIController.generated.h"

class UBlackboardComponent;
class UBlackboardData;
class UBehaviorTree;

UCLASS()
class RAND_API ARANDNPCAIController : public AAIController
{
	GENERATED_BODY()

public:
	ARANDNPCAIController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolRadius = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float GreetingRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Idle")
	float IdleMinSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Idle")
	float IdleMaxSeconds = 5.0f;

	UFUNCTION(BlueprintCallable, Category = "AI")
	void NotifyCrimeWitnessed();

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

	enum class EAIState : uint8 { Idle, Patrol, Flee, Pursue };
	EAIState State = EAIState::Idle;

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
	void UpdateWantedPursuit();
	void AttackPlayer();
	void ShowLine(const FText& Line);
};
