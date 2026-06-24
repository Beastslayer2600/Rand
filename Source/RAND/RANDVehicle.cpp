// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDVehicle.h"
#include "RANDCharacter.h"
#include "WantedComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

ARANDVehicle::ARANDVehicle()
{
	WantedComponent = CreateDefaultSubobject<UWantedComponent>(TEXT("WantedComponent"));

	// Exit input assets authored in code so the vehicle is playable with no
	// editor setup, mirroring the character's approach.
	VehicleMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("VehicleMappingContext"));

	ExitAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_ExitVehicle"));
	ExitAction->ValueType = EInputActionValueType::Boolean;
}

// --- IInteractable ----------------------------------------------------------

FText ARANDVehicle::GetInteractionPrompt_Implementation() const
{
	return NSLOCTEXT("RANDVehicle", "EnterVehicle", "Enter Vehicle");
}

bool ARANDVehicle::CanInteract_Implementation(const AActor* /*Interactor*/) const
{
	// Can't climb into a car that's already occupied.
	return !IsOccupied();
}

void ARANDVehicle::OnInteract_Implementation(AActor* Interactor)
{
	ARANDCharacter* Character = Cast<ARANDCharacter>(Interactor);
	if (!Character || IsOccupied())
	{
		return;
	}

	AController* Controller = Character->GetController();
	if (!Controller)
	{
		return;
	}

	// Remember who was driving André so we can hand control back on exit.
	DriverCharacter = Character;
	SavedController = Controller;

	// Theft: entering an unowned vehicle is grand theft auto.
	if (!bOwned)
	{
		if (UWantedComponent* DriverHeat = Character->GetWantedComponent())
		{
			DriverHeat->AddHeat(EAgency::SAPS, TheftHeat);
		}
	}

	// Park André: hide him and take him out of the world while he's seated.
	Character->GetMesh()->SetVisibility(false, /*bPropagateToChildren=*/true);
	Character->SetActorEnableCollision(false);
	Character->DisableInput(Cast<APlayerController>(Controller));

	// Possess the vehicle with André's controller.
	Controller->Possess(this);

	// Build the exit key mapping for whoever is now driving.
	VehicleMappingContext->UnmapAll();
	VehicleMappingContext->MapKey(ExitAction, EKeys::F);

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(VehicleMappingContext, 0);
		}
	}
}

// --- Input ------------------------------------------------------------------

void ARANDVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(ExitAction, ETriggerEvent::Started, this, &ARANDVehicle::ExitVehicle);
	}
}

void ARANDVehicle::ExitVehicle()
{
	if (!DriverCharacter || !SavedController)
	{
		return;
	}

	AController* Controller = SavedController;

	// Tear down the exit mapping context.
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(VehicleMappingContext);
		}
	}

	// Hand the controller back to André.
	Controller->UnPossess();
	Controller->Possess(DriverCharacter);

	// Drop André at the driver door, in world space.
	const FVector DropLocation = GetActorTransform().TransformPosition(DriverDoorOffset);
	DriverCharacter->SetActorLocation(DropLocation, /*bSweep=*/true);

	// Restore André.
	DriverCharacter->GetMesh()->SetVisibility(true, /*bPropagateToChildren=*/true);
	DriverCharacter->SetActorEnableCollision(true);
	DriverCharacter->EnableInput(Cast<APlayerController>(Controller));

	DriverCharacter = nullptr;
	SavedController = nullptr;
}
