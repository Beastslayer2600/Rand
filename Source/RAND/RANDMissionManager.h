// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDMissionManager.generated.h"

/** Lifecycle state of a mission. */
UENUM(BlueprintType)
enum class EMissionStatus : uint8
{
	Inactive	UMETA(DisplayName = "Inactive"),
	Active		UMETA(DisplayName = "Active"),
	Complete	UMETA(DisplayName = "Complete"),
	Failed		UMETA(DisplayName = "Failed"),
};

/** A single objective within a mission. */
USTRUCT(BlueprintType)
struct FRANDObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FText ObjectiveText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	bool bIsComplete = false;

	/** Optional objectives don't block mission completion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	bool bIsOptional = false;
};

/** A mission: an identifier, display name, ordered objectives, and status. */
USTRUCT(BlueprintType)
struct FRANDMission
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FName MissionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FText MissionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	TArray<FRANDObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	EMissionStatus Status = EMissionStatus::Inactive;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStarted, FName, MissionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveComplete, FName, MissionID, int32, ObjectiveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionComplete, FName, MissionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionFailed, FName, MissionID);

/**
 * URANDMissionManager — lives on ARANDGameMode and tracks all missions.
 *
 * Missions are registered (definition + objectives), then driven via
 * StartMission / CompleteObjective / CompleteMission / FailMission. Completing
 * the last required objective auto-completes the mission. One mission is
 * "active" at a time for HUD display.
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDMissionManager : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDMissionManager();

	/** Resolves the manager from the active game mode. */
	static URANDMissionManager* Get(const UObject* WorldContext);

	// --- Registration / flow ------------------------------------------------

	/** Add (or replace) a mission definition keyed by its MissionID. */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void RegisterMission(const FRANDMission& Mission);

	/** Marks a registered mission Active and becomes the displayed mission. */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	bool StartMission(FName MissionID);

	/** Marks one objective complete; auto-completes the mission when all required objectives are done. */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	bool CompleteObjective(FName MissionID, int32 ObjectiveIndex);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	bool CompleteMission(FName MissionID);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	bool FailMission(FName MissionID);

	// --- Queries ------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Mission")
	bool GetMission(FName MissionID, FRANDMission& OutMission) const;

	UFUNCTION(BlueprintPure, Category = "Mission")
	bool GetActiveMission(FRANDMission& OutMission) const;

	UFUNCTION(BlueprintPure, Category = "Mission")
	FName GetActiveMissionID() const { return ActiveMissionID; }

	// --- Delegates ----------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FOnMissionStarted OnMissionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FOnObjectiveComplete OnObjectiveComplete;

	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FOnMissionComplete OnMissionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FOnMissionFailed OnMissionFailed;

private:
	UPROPERTY()
	TMap<FName, FRANDMission> Missions;

	FName ActiveMissionID = NAME_None;

	/** True when every non-optional objective of the mission is complete. */
	static bool AreRequiredObjectivesComplete(const FRANDMission& Mission);
};
