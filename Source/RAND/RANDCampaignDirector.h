// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueComponent.h"
#include "RANDCareerComponent.h"
#include "RANDCampaignDirector.generated.h"

class ARANDCharacter_NPC;
class AActor;

UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDCampaignDirector : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDCampaignDirector();

	static const FName Mission2ID;
	static const FName Mission3ID;
	static const FName Mission4ID;
	static const FName Mission5ID;

	UFUNCTION(BlueprintPure, Category = "Campaign")
	int32 GetAct1Index() const { return ActIndex; }

protected:
	virtual void BeginPlay() override;

private:
	int32 ActIndex = 1;
	bool bSeeded = false;

	UPROPERTY()
	TObjectPtr<ARANDCharacter_NPC> ActiveNPC;

	UPROPERTY()
	TObjectPtr<AActor> ActiveTriggerActor;

	FTimerHandle KickTimer;
	FTimerHandle MessageTimer;

	UFUNCTION()
	void Kickoff();

	UFUNCTION()
	void HandleMissionComplete(FName MissionID);

	UFUNCTION()
	void HandlePhoneOption(FName ActionId);

	void SeedStartingLife();
	void EnsureConsultationExists();
	void BindPhone();

	void StartMission2();
	void StartMission3();
	void StartMission4();
	void StartMission5();
	void FinishCampaign();

	void SpawnTalkObjective(const FString& NPCName, const FText& Line, FName CompleteAs);

	UFUNCTION()
	void CompleteActiveTalk();

	UFUNCTION()
	void HandleDialogueStarted(const FRANDDialogueLine& Line);

	void Send(const FString& Sender, const FText& Text);
	void SendChoice(const FString& Sender, const FText& Text,
		const FText& ALabel, FName AId, const FText& BLabel, FName BId);
};
