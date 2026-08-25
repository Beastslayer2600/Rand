// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RANDCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UInteractionComponent;
class UHealthComponent;
class UWantedComponent;
class URANDEconomyComponent;
class URANDBusinessManager;
class URANDCareerComponent;
class URANDReputationComponent;
class URANDCombatComponent;

UCLASS()
class RAND_API ARANDCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARANDCharacter();

	UFUNCTION(BlueprintPure, Category = "Wanted")
	UWantedComponent* GetWantedComponent() const { return WantedComponent; }

	UFUNCTION(BlueprintPure, Category = "Health")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	UFUNCTION(BlueprintPure, Category = "Economy")
	URANDEconomyComponent* GetEconomyComponent() const { return EconomyComponent; }

	UFUNCTION(BlueprintPure, Category = "Business")
	URANDBusinessManager* GetBusinessManager() const { return BusinessManager; }

	UFUNCTION(BlueprintPure, Category = "Career")
	URANDCareerComponent* GetCareerComponent() const { return CareerComponent; }

	UFUNCTION(BlueprintPure, Category = "Reputation")
	URANDReputationComponent* GetReputationComponent() const { return ReputationComponent; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	URANDCombatComponent* GetCombatComponent() const { return CombatComponent; }

	UPROPERTY(BlueprintReadWrite, Category = "Story")
	bool bAcceptedBribe = false;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> GoDarkAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FileTaxesAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 650.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wanted", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWantedComponent> WantedComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URANDEconomyComponent> EconomyComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Business", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URANDBusinessManager> BusinessManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Career", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URANDCareerComponent> CareerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reputation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URANDReputationComponent> ReputationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URANDCombatComponent> CombatComponent;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);
	void HandleFire(const FInputActionValue& Value);
	void HandleGoDark(const FInputActionValue& Value);
	void HandleFileTaxes(const FInputActionValue& Value);

private:
	void ConfigureInputMappings();
};
