// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDSaveGameManager.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDSaveGameManager : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDSaveGameManager();

	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveGame();

	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadGame();

protected:
	virtual void BeginPlay() override;

private:
	void SetupInput();

	UFUNCTION()
	void HandleManualSave();

	UFUNCTION()
	void HandleManualLoad();

	UFUNCTION()
	void HandleHourPassed(int32 Day, int32 Hour);

	UPROPERTY()
	TObjectPtr<UInputMappingContext> SaveMappingContext;

	UPROPERTY()
	TObjectPtr<UInputAction> SaveAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LoadAction;

	FTimerHandle InputSetupTimer;
};
