// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WantedComponent.h"
#include "RANDHUDWidget.generated.h"

class ARANDCharacter;
class UHealthComponent;
class UInteractionComponent;
class URANDTimeComponent;
class URANDMissionManager;
class URANDMinimapWidget;
class UProgressBar;
class UTextBlock;
class UVerticalBox;

UCLASS()
class RAND_API URANDHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	URANDHUDWidget(const FObjectInitializer& ObjectInitializer);

	void BindToCharacter(ARANDCharacter* Character);
	void SetPhoneNotification(bool bHasUnread);
	static FText HeatLevelToText(EHeatLevel Level);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

	UPROPERTY() TObjectPtr<UProgressBar> HealthBar;
	UPROPERTY() TObjectPtr<UTextBlock> HealthText;
	UPROPERTY() TObjectPtr<UTextBlock> InteractionText;
	UPROPERTY() TObjectPtr<UTextBlock> TimeText;
	UPROPERTY() TObjectPtr<UTextBlock> StatusText;
	UPROPERTY() TObjectPtr<UVerticalBox> MissionBox;
	UPROPERTY() TObjectPtr<UTextBlock> PhoneNotificationText;
	UPROPERTY() TObjectPtr<URANDMinimapWidget> Minimap;
	UPROPERTY() TArray<TObjectPtr<UProgressBar>> HeatBars;
	UPROPERTY() TArray<TObjectPtr<UTextBlock>> HeatLevelTexts;

private:
	static constexpr int32 AgencyCount = 3;
	static constexpr int32 MaxHeatLevel = 4;

	TWeakObjectPtr<UHealthComponent> BoundHealth;
	TWeakObjectPtr<UInteractionComponent> BoundInteraction;
	TWeakObjectPtr<UWantedComponent> BoundWanted;
	TWeakObjectPtr<URANDTimeComponent> BoundTime;
	TWeakObjectPtr<URANDMissionManager> BoundMissions;

	class UHorizontalBox* BuildHeatRow(int32 AgencyIdx, const TCHAR* DisplayName, const FLinearColor& Color);

	UFUNCTION() void HandleHealthChanged(float NewHealth, float Delta);
	UFUNCTION() void HandleHeatChanged(EAgency Agency, EHeatLevel OldLevel, EHeatLevel NewLevel);
	UFUNCTION() void HandleTargetChanged(AActor* NewTarget);
	UFUNCTION() void HandleMinutePassed(int32 Day, int32 Hour, int32 Minute);
	UFUNCTION() void HandleMissionStarted(FName MissionID);
	UFUNCTION() void HandleObjectiveComplete(FName MissionID, int32 ObjectiveIndex);
	UFUNCTION() void HandleMissionComplete(FName MissionID);
	UFUNCTION() void HandleMissionFailed(FName MissionID);

	void UpdateMissionDisplay();
	void UpdateStatusLine();
	void RefreshAll();
};
