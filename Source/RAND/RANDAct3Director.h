// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDAct3Director.generated.h"

UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDAct3Director : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDAct3Director();

	static const FName Mission9ID;
	static const FName Mission10ID;
	static const FName Mission11ID;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION() void HandleMissionComplete(FName MissionID);
	UFUNCTION() void HandlePhoneOption(FName ActionId);
	void Bind();
	void StartMission9();
	void StartMission10();
	void StartMission11();
	void Finish();
	void Send(const FString& Sender, const FText& Text);
	void SendChoice(const FString& Sender, const FText& Text, const FText& A, FName AId, const FText& B, FName BId);

	FTimerHandle BindTimer;
};
