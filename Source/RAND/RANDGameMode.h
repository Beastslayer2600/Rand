// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RANDGameMode.generated.h"

class URANDTimeComponent;
class URANDSaveGameManager;
class URANDMissionManager;
class URANDWeatherComponent;
class URANDCampaignDirector;
class URANDWantedResponseComponent;
class URANDSARSComponent;
class URANDCheatComponent;
class URANDSideHustleComponent;
class URANDWorldSeeder;
class URANDAct3Director;

UCLASS()
class RAND_API ARANDGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARANDGameMode();

	UFUNCTION(BlueprintPure, Category = "Time") URANDTimeComponent* GetTimeComponent() const { return TimeComponent; }
	UFUNCTION(BlueprintPure, Category = "Save") URANDSaveGameManager* GetSaveManager() const { return SaveManager; }
	UFUNCTION(BlueprintPure, Category = "Mission") URANDMissionManager* GetMissionManager() const { return MissionManager; }
	UFUNCTION(BlueprintPure, Category = "Weather") URANDWeatherComponent* GetWeatherComponent() const { return WeatherComponent; }
	UFUNCTION(BlueprintPure, Category = "Campaign") URANDCampaignDirector* GetCampaignDirector() const { return CampaignDirector; }
	UFUNCTION(BlueprintPure, Category = "Wanted") URANDWantedResponseComponent* GetWantedResponse() const { return WantedResponse; }
	UFUNCTION(BlueprintPure, Category = "SARS") URANDSARSComponent* GetSARSComponent() const { return SARSComponent; }
	UFUNCTION(BlueprintPure, Category = "Debug") URANDCheatComponent* GetCheatComponent() const { return CheatComponent; }
	UFUNCTION(BlueprintPure, Category = "Jobs") URANDSideHustleComponent* GetSideHustle() const { return SideHustle; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time", meta = (AllowPrivateAccess = "true")) TObjectPtr<URANDTimeComponent> TimeComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save", meta = (AllowPrivateAccess = "true")) TObjectPtr<URANDSaveGameManager> SaveManager;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission", meta = (AllowPrivateAccess = "true")) TObjectPtr<URANDMissionManager> MissionManager;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather", meta = (AllowPrivateAccess = "true")) TObjectPtr<URANDWeatherComponent> WeatherComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Campaign", meta = (AllowPrivateAccess = "true")) TObjectPtr<URANDCampaignDirector> CampaignDirector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wanted", meta = (AllowPrivateAccess = "true")) TObjectPtr<URANDWantedResponseComponent> WantedResponse;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SARS", meta = (AllowPrivateAccess = "true")) TObjectPtr<URANDSARSComponent> SARSComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true")) TObjectPtr<URANDCheatComponent> CheatComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jobs", meta = (AllowPrivateAccess = "true")) TObjectPtr<URANDSideHustleComponent> SideHustle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World", meta = (AllowPrivateAccess = "true")) TObjectPtr<URANDWorldSeeder> WorldSeeder;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Campaign", meta = (AllowPrivateAccess = "true")) TObjectPtr<URANDAct3Director> Act3Director;
};
