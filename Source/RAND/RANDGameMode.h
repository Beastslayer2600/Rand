// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RANDGameMode.generated.h"

class URANDTimeComponent;
class URANDSaveGameManager;

/**
 * ARANDGameMode — default rules for RAND. Spawns André (ARANDCharacter) as
 * the player pawn. Single-player; expands later with save/load and the
 * economy/heat systems described in the GDD.
 */
UCLASS()
class RAND_API ARANDGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARANDGameMode();

	UFUNCTION(BlueprintPure, Category = "Time")
	URANDTimeComponent* GetTimeComponent() const { return TimeComponent; }

	UFUNCTION(BlueprintPure, Category = "Save")
	URANDSaveGameManager* GetSaveManager() const { return SaveManager; }

private:
	/** Authoritative in-game clock for the session. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URANDTimeComponent> TimeComponent;

	/** Save/load for the session (F5 + hourly auto-save). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URANDSaveGameManager> SaveManager;
};
