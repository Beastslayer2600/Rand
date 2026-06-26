// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void URANDAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwningCharacter)
	{
		MovementComponent = OwningCharacter->GetCharacterMovement();
	}
}

void URANDAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter || !MovementComponent)
	{
		return;
	}

	const FVector Velocity = OwningCharacter->GetVelocity();
	GroundSpeed = Velocity.Size2D();
	bIsInAir = MovementComponent->IsFalling();
	bShouldMove = GroundSpeed > 3.0f && !MovementComponent->GetCurrentAcceleration().IsNearlyZero();

	// Signed angle between facing and velocity in the horizontal plane.
	// (Equivalent to UKismetAnimationLibrary::CalculateDirection, inlined to
	// avoid a dependency on the AnimGraphRuntime module.)
	if (!Velocity.IsNearlyZero())
	{
		const FRotator BaseRotation = OwningCharacter->GetActorRotation();
		const FMatrix RotMatrix = FRotationMatrix(BaseRotation);
		const FVector Forward = RotMatrix.GetScaledAxis(EAxis::X);
		const FVector Right = RotMatrix.GetScaledAxis(EAxis::Y);
		const FVector NormalizedVel = Velocity.GetSafeNormal2D();

		const float ForwardCos = FVector::DotProduct(Forward, NormalizedVel);
		const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(ForwardCos));
		const float RightDot = FVector::DotProduct(Right, NormalizedVel);
		Direction = (RightDot < 0.0f) ? -AngleDegrees : AngleDegrees;
	}
	else
	{
		Direction = 0.0f;
	}
}
