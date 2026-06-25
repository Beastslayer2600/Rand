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

/**
 * ARANDCharacter — André Venter, the player-controlled protagonist.
 *
 * Third-person controller built on Enhanced Input. Movement is deliberately
 * weighty (GTA IV reference in the GDD): a measured walk speed with a sprint
 * modifier rather than twitchy arcade movement. Input bindings are authored
 * entirely in C++ so the project compiles and plays without any editor-side
 * asset setup; designers can later override the mapping context as a data asset.
 */
UCLASS()
class RAND_API ARANDCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARANDCharacter();

	/** Accessors for the gameplay systems André owns. */
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

	// --- Story flags --------------------------------------------------------

	/** Set by Mission 1 (The Consultation): did André take the tender bribe? */
	UPROPERTY(BlueprintReadWrite, Category = "Story")
	bool bAcceptedBribe = false;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// --- Camera ---------------------------------------------------------

	/** Boom positioning the camera behind André. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Follow camera at the end of the boom. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	// --- Enhanced Input -------------------------------------------------

	/** Default mapping context applied when this character is possessed. */
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

	// --- Movement tuning ------------------------------------------------

	/** Ground speed for a normal walk (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 400.0f;

	/** Ground speed while sprinting (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 650.0f;

	// --- Systems --------------------------------------------------------

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

	// --- Input handlers -------------------------------------------------

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);

private:
	/** Builds the key->action mappings on the default mapping context. */
	void ConfigureInputMappings();
};
