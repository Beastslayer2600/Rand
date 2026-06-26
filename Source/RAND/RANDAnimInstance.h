// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RANDAnimInstance.generated.h"

class ACharacter;
class UCharacterMovementComponent;

/**
 * URANDAnimInstance — locomotion driver for André's mannequin (and later his
 * MetaHuman). Reads the owning character's CharacterMovementComponent every
 * frame and exposes the values an AnimGraph needs to blend idle/walk/run and
 * switch to an air state. Authored in C++ so the logic is version-controlled;
 * the AnimGraph that consumes these values is wired in ABP_Andre.
 *
 * Replaces the MoverExamples ABP_Manny, which is coupled to the Mover plugin
 * and does not drive a standard CharacterMovement-based pawn.
 */
UCLASS()
class RAND_API URANDAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	/** Horizontal speed (cm/s). Drives the idle/walk/run blendspace. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float GroundSpeed = 0.0f;

	/** Signed angle (-180..180) of velocity relative to facing, for strafing. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Direction = 0.0f;

	/** True while falling or jumping. Drives the air/jump state. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsInAir = false;

	/** True when meaningfully moving. Gates idle vs. locomotion. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bShouldMove = false;

private:
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwningCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> MovementComponent;
};
