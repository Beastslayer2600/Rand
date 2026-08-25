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
#include "Engine/World.h"
#include "TimerManager.h"

#include "InteractionComponent.h"
#include "HealthComponent.h"
#include "WantedComponent.h"
#include "EconomyComponent.h"
#include "BusinessManager.h"
#include "RANDCareerComponent.h"
#include "RANDReputationComponent.h"
#include "RANDCombatComponent.h"
#include "RANDInventoryComponent.h"
#include "RANDSARSComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"
#include "InputActionValue.h"

ARANDCharacter::ARANDCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	Movement->MaxWalkSpeed = WalkSpeed;
	Movement->MinAnalogWalkSpeed = 20.0f;
	Movement->BrakingDecelerationWalking = 1500.0f;
	Movement->JumpZVelocity = 420.0f;
	Movement->AirControl = 0.2f;

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> PlaceholderMesh(
		TEXT("/MoverExamples/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	if (PlaceholderMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(PlaceholderMesh.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> LocomotionABP(
		TEXT("/Game/Characters/ABP_Andre.ABP_Andre_C"));
	if (LocomotionABP.Succeeded())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(LocomotionABP.Class);
	}

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	DefaultMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("DefaultMappingContext"));

	MoveAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Move"));
	MoveAction->ValueType = EInputActionValueType::Axis2D;

	LookAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;

	JumpAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Jump"));
	JumpAction->ValueType = EInputActionValueType::Boolean;

	SprintAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Sprint"));
	SprintAction->ValueType = EInputActionValueType::Boolean;

	FireAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Fire"));
	FireAction->ValueType = EInputActionValueType::Boolean;

	GoDarkAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_GoDark"));
	GoDarkAction->ValueType = EInputActionValueType::Boolean;

	FileTaxesAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_FileTaxes"));
	FileTaxesAction->ValueType = EInputActionValueType::Boolean;

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	HealthComponent      = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	WantedComponent      = CreateDefaultSubobject<UWantedComponent>(TEXT("WantedComponent"));
	EconomyComponent     = CreateDefaultSubobject<URANDEconomyComponent>(TEXT("EconomyComponent"));
	BusinessManager      = CreateDefaultSubobject<URANDBusinessManager>(TEXT("BusinessManager"));
	CareerComponent      = CreateDefaultSubobject<URANDCareerComponent>(TEXT("CareerComponent"));
	ReputationComponent  = CreateDefaultSubobject<URANDReputationComponent>(TEXT("ReputationComponent"));
	CombatComponent      = CreateDefaultSubobject<URANDCombatComponent>(TEXT("CombatComponent"));
	InventoryComponent   = CreateDefaultSubobject<URANDInventoryComponent>(TEXT("InventoryComponent"));
}

void ARANDCharacter::ConfigureInputMappings()
{
	if (!DefaultMappingContext) return;

	auto MakeNegate = [this]() { return NewObject<UInputModifierNegate>(this); };
	auto MakeSwizzleYXZ = [this]()
	{
		UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(this);
		Swizzle->Order = EInputAxisSwizzle::YXZ;
		return Swizzle;
	};

	{ FEnhancedActionKeyMapping& M = DefaultMappingContext->MapKey(MoveAction, EKeys::W); M.Modifiers.Add(MakeSwizzleYXZ()); }
	{ FEnhancedActionKeyMapping& M = DefaultMappingContext->MapKey(MoveAction, EKeys::S); M.Modifiers.Add(MakeNegate()); M.Modifiers.Add(MakeSwizzleYXZ()); }
	DefaultMappingContext->MapKey(MoveAction, EKeys::D);
	{ FEnhancedActionKeyMapping& M = DefaultMappingContext->MapKey(MoveAction, EKeys::A); M.Modifiers.Add(MakeNegate()); }
	DefaultMappingContext->MapKey(LookAction, EKeys::MouseX);
	{ FEnhancedActionKeyMapping& M = DefaultMappingContext->MapKey(LookAction, EKeys::MouseY); M.Modifiers.Add(MakeNegate()); M.Modifiers.Add(MakeSwizzleYXZ()); }
	DefaultMappingContext->MapKey(JumpAction, EKeys::SpaceBar);
	DefaultMappingContext->MapKey(SprintAction, EKeys::LeftShift);
	DefaultMappingContext->MapKey(FireAction, EKeys::LeftMouseButton);
	DefaultMappingContext->MapKey(GoDarkAction, EKeys::F6);
	DefaultMappingContext->MapKey(FileTaxesAction, EKeys::F7);
}

void ARANDCharacter::BeginPlay()
{
	Super::BeginPlay();
	RespawnLocation = GetActorLocation();
	ConfigureInputMappings();
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &ARANDCharacter::HandleDeath);
	}
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ARANDCharacter::HandleDeath()
{
	if (EconomyComponent)
	{
		EconomyComponent->DeductFunds(FMath::Min(5000.0f, EconomyComponent->GetBalance()), TEXT("Hospital / fixer"));
	}
	if (WantedComponent)
	{
		WantedComponent->AddHeat(EAgency::SAPS, 10.0f);
	}
	if (UWorld* World = GetWorld())
	{
		FTimerHandle ReviveTimer;
		World->GetTimerManager().SetTimer(ReviveTimer, [this]()
		{
			SetActorLocation(RespawnLocation);
			if (HealthComponent) HealthComponent->Revive();
		}, 2.5f, false);
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
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &ARANDCharacter::HandleFire);
		EnhancedInput->BindAction(GoDarkAction, ETriggerEvent::Started, this, &ARANDCharacter::HandleGoDark);
		EnhancedInput->BindAction(FileTaxesAction, ETriggerEvent::Started, this, &ARANDCharacter::HandleFileTaxes);
	}
}

void ARANDCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller || Axis.IsNearlyZero()) return;
	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Axis.X);
}

void ARANDCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller) return;
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ARANDCharacter::StartSprint(const FInputActionValue&) { GetCharacterMovement()->MaxWalkSpeed = SprintSpeed; }
void ARANDCharacter::StopSprint(const FInputActionValue&) { GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; }
void ARANDCharacter::HandleFire(const FInputActionValue&) { if (CombatComponent) CombatComponent->Fire(); }
void ARANDCharacter::HandleGoDark(const FInputActionValue&) { if (CareerComponent) CareerComponent->GoDark(); }
void ARANDCharacter::HandleFileTaxes(const FInputActionValue&) { if (URANDSARSComponent* SARS = URANDSARSComponent::Get(this)) SARS->FileReturn(); }
