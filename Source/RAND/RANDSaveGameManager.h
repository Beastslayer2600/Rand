// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDSaveGameManager.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * URANDSaveGameManager — lives on ARANDGameMode and owns save/load for the
 * session. Serialises André's economy, businesses, heat, the game clock, and
 * his location into URANDSaveGame's single slot.
 *
 * Triggers: manual save on F5, and an auto-save every in-game hour
 * (TimeComponent::OnHourPassed).
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDSaveGameManager : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDSaveGameManager();

	/** Snapshot the current session into the save slot. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveGame();

	/** Restore the session from the save slot, if one exists. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadGame();

protected:
	virtual void BeginPlay() override;

private:
	/** Binds F5 -> manual save on the local player (deferred until input exists). */
	void SetupInput();

	UFUNCTION()
	void HandleManualSave();

	UFUNCTION()
	void HandleHourPassed(int32 Day, int32 Hour);

	UPROPERTY()
	TObjectPtr<UInputMappingContext> SaveMappingContext;

	UPROPERTY()
	TObjectPtr<UInputAction> SaveAction;

	FTimerHandle InputSetupTimer;
};
