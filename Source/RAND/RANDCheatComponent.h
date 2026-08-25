// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDCheatComponent.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDCheatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDCheatComponent();

protected:
	virtual void BeginPlay() override;

private:
	void SetupInput();

	UFUNCTION() void GiveCash();
	UFUNCTION() void ArmPlayer();
	UFUNCTION() void SkipObjective();
	UFUNCTION() void ClearHeat();
	UFUNCTION() void PhoneChoice1();
	UFUNCTION() void PhoneChoice2();
	UFUNCTION() void ReloadWeapon();
	void PickPhoneOption(int32 Index);

	UPROPERTY() TObjectPtr<UInputMappingContext> CheatMappingContext;
	UPROPERTY() TObjectPtr<UInputAction> CashAction;
	UPROPERTY() TObjectPtr<UInputAction> ArmAction;
	UPROPERTY() TObjectPtr<UInputAction> SkipAction;
	UPROPERTY() TObjectPtr<UInputAction> HeatAction;
	UPROPERTY() TObjectPtr<UInputAction> Choice1Action;
	UPROPERTY() TObjectPtr<UInputAction> Choice2Action;
	UPROPERTY() TObjectPtr<UInputAction> ReloadAction;

	FTimerHandle InputSetupTimer;
};
