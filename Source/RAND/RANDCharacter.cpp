// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

#include "InteractionComponent.h"
#include "HealthComponent.h"
#include "WantedComponent.h"
#include "EconomyComponent.h"
#include "BusinessManager.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"
#include "InputActionValue.h"

ARANDCharacter::ARANDCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Collision capsule sized for an adult male.
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// The controller's yaw drives movement direction, not the mesh's facing.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;            // Face the direction of travel.
	Movement->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // Deliberate turn — weighty, not snappy.
	Movement->MaxWalkSpeed = WalkSpeed;
	Movement->MinAnalogWalkSpeed = 20.0f;
	Movement->BrakingDecelerationWalking = 1500.0f;
	Movement->JumpZVelocity = 420.0f;
	Movement->AirControl = 0.2f;

	// Placeholder visual: the engine "Manny" mannequin so movement is visible
	// during play-tests. André's real look is a MetaHuman (per GDD) — this is
	// a throwaway stand-in in reference pose (no locomotion anims yet).
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> PlaceholderMesh(
		TEXT("/MoverExamples/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	if (PlaceholderMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(PlaceholderMesh.Object);
	}

	// Locomotion AnimBP — ABP_Andre, parented to URANDAnimInstance (our C++
	// driver). Replaces the MoverExamples ABP_Manny, which is Mover-coupled and
	// won't animate a CharacterMovement-based pawn. The AnimGraph blends the
	// Manny idle/walk/run/jump anims off GroundSpeed + bIsInAir.
	static ConstructorHelpers::FClassFinder<UAnimInstance> LocomotionABP(
		TEXT("/Game/Characters/ABP_Andre.ABP_Andre_C"));
	if (LocomotionABP.Succeeded())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(LocomotionABP.Class);
	}

	// Camera boom: trails behind André and collides with world geometry.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Author the Enhanced Input assets in code so the project is playable with
	// no editor setup. These can be replaced by designer-authored data assets.
	DefaultMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("DefaultMappingContext"));

	MoveAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Move"));
	MoveAction->ValueType = EInputActionValueType::Axis2D;

	LookAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;

	JumpAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Jump"));
	JumpAction->ValueType = EInputActionValueType::Boolean;

	SprintAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Sprint"));
	SprintAction->ValueType = EInputActionValueType::Boolean;

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	HealthComponent      = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	WantedComponent      = CreateDefaultSubobject<UWantedComponent>(TEXT("WantedComponent"));
	EconomyComponent     = CreateDefaultSubobject<URANDEconomyComponent>(TEXT("EconomyComponent"));
	BusinessManager      = CreateDefaultSubobject<URANDBusinessManager>(TEXT("BusinessManager"));
}

void ARANDCharacter::ConfigureInputMappings()
{
	if (!DefaultMappingContext)
	{
		return;
	}

	// WASD -> 2D move axis. A key contributes to X by default; we negate and
	// swizzle (X<->Y) to compose forward/back on Y and strafe on X.
	auto MakeNegate = [this]() { return NewObject<UInputModifierNegate>(this); };
	auto MakeSwizzleYXZ = [this]()
	{
		UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(this);
		Swizzle->Order = EInputAxisSwizzle::YXZ;
		return Swizzle;
	};

	// Forward (W): swizzle X->Y, positive.
	{
		FEnhancedActionKeyMapping& M = DefaultMappingContext->MapKey(MoveAction, EKeys::W);
		M.Modifiers.Add(MakeSwizzleYXZ());
	}
	// Back (S): swizzle X->Y, negated.
	{
		FEnhancedActionKeyMapping& M = DefaultMappingContext->MapKey(MoveAction, EKeys::S);
		M.Modifiers.Add(MakeNegate());
		M.Modifiers.Add(MakeSwizzleYXZ());
	}
	// Right (D): positive X.
	{
		DefaultMappingContext->MapKey(MoveAction, EKeys::D);
	}
	// Left (A): negated X.
	{
		FEnhancedActionKeyMapping& M = DefaultMappingContext->MapKey(MoveAction, EKeys::A);
		M.Modifiers.Add(MakeNegate());
	}

	// Mouse look. MouseX -> yaw (X). MouseY -> pitch (Y), swizzled and negated
	// so pushing the mouse forward looks up.
	{
		DefaultMappingContext->MapKey(LookAction, EKeys::MouseX);
	}
	{
		FEnhancedActionKeyMapping& M = DefaultMappingContext->MapKey(LookAction, EKeys::MouseY);
		M.Modifiers.Add(MakeNegate());
		M.Modifiers.Add(MakeSwizzleYXZ());
	}

	// Jump (Space) and Sprint (Left Shift).
	DefaultMappingContext->MapKey(JumpAction, EKeys::SpaceBar);
	DefaultMappingContext->MapKey(SprintAction, EKeys::LeftShift);
}

void ARANDCharacter::BeginPlay()
{
	Super::BeginPlay();

	ConfigureInputMappings();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ARANDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARANDCharacter::Move);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARANDCharacter::Look);

		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ARANDCharacter::StartSprint);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ARANDCharacter::StopSprint);
	}
}

void ARANDCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller || Axis.IsNearlyZero())
	{
		return;
	}

	// Movement is relative to the camera yaw, projected onto the ground plane.
	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);
}

void ARANDCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller)
	{
		return;
	}

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ARANDCharacter::StartSprint(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ARANDCharacter::StopSprint(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}
