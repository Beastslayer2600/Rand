// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDMissionManager.h"

#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"

URANDMissionManager::URANDMissionManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

URANDMissionManager* URANDMissionManager::Get(const UObject* WorldContext)
{
	if (!WorldContext)
	{
		return nullptr;
	}
	const UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		return nullptr;
	}
	if (AGameModeBase* GameMode = World->GetAuthGameMode())
	{
		return GameMode->FindComponentByClass<URANDMissionManager>();
	}
	return nullptr;
}

void URANDMissionManager::RegisterMission(const FRANDMission& Mission)
{
	Missions.Add(Mission.MissionID, Mission);
}

bool URANDMissionManager::StartMission(FName MissionID)
{
	FRANDMission* Mission = Missions.Find(MissionID);
	if (!Mission)
	{
		return false;
	}

	Mission->Status = EMissionStatus::Active;
	ActiveMissionID = MissionID;
	OnMissionStarted.Broadcast(MissionID);
	return true;
}

bool URANDMissionManager::CompleteObjective(FName MissionID, int32 ObjectiveIndex)
{
	FRANDMission* Mission = Missions.Find(MissionID);
	if (!Mission || !Mission->Objectives.IsValidIndex(ObjectiveIndex))
	{
		return false;
	}
	if (Mission->Status != EMissionStatus::Active)
	{
		return false;
	}
	if (Mission->Objectives[ObjectiveIndex].bIsComplete)
	{
		return true; // Already done — idempotent.
	}

	Mission->Objectives[ObjectiveIndex].bIsComplete = true;
	OnObjectiveComplete.Broadcast(MissionID, ObjectiveIndex);

	// Finishing the last required objective completes the mission.
	if (AreRequiredObjectivesComplete(*Mission))
	{
		CompleteMission(MissionID);
	}
	return true;
}

bool URANDMissionManager::CompleteMission(FName MissionID)
{
	FRANDMission* Mission = Missions.Find(MissionID);
	if (!Mission)
	{
		return false;
	}

	Mission->Status = EMissionStatus::Complete;
	if (ActiveMissionID == MissionID)
	{
		ActiveMissionID = NAME_None;
	}
	OnMissionComplete.Broadcast(MissionID);
	return true;
}

bool URANDMissionManager::FailMission(FName MissionID)
{
	FRANDMission* Mission = Missions.Find(MissionID);
	if (!Mission)
	{
		return false;
	}

	Mission->Status = EMissionStatus::Failed;
	if (ActiveMissionID == MissionID)
	{
		ActiveMissionID = NAME_None;
	}
	OnMissionFailed.Broadcast(MissionID);
	return true;
}

bool URANDMissionManager::GetMission(FName MissionID, FRANDMission& OutMission) const
{
	if (const FRANDMission* Mission = Missions.Find(MissionID))
	{
		OutMission = *Mission;
		return true;
	}
	return false;
}

bool URANDMissionManager::GetActiveMission(FRANDMission& OutMission) const
{
	if (ActiveMissionID.IsNone())
	{
		return false;
	}
	return GetMission(ActiveMissionID, OutMission);
}

bool URANDMissionManager::AreRequiredObjectivesComplete(const FRANDMission& Mission)
{
	for (const FRANDObjective& Objective : Mission.Objectives)
	{
		if (!Objective.bIsOptional && !Objective.bIsComplete)
		{
			return false;
		}
	}
	return true;
}
