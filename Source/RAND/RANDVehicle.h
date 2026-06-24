// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "IInteractable.h"
#include "RANDVehicle.generated.h"

class UWantedComponent;
class ARANDCharacter;

/**
 * ARANDVehicle — base class for every drivable vehicle in RAND.
 *
 * Built on Chaos AWheeledVehiclePawn and implements IInteractable so André can
 * walk up and press E to get in. Mounting possesses the vehicle with André's
 * player controller, hides André's character, and remembers him so he can be
 * re-spawned on exit (F).
 *
 * Theft heat: if bOwned is false, entering the vehicle is grand theft auto —
 * the driver's UWantedComponent gets a SAPS heat bump on entry.
 */
UCLASS()
class RAND_API ARANDVehicle : public AWheeledVehiclePawn, public IInteractable
{
	GENERATED_BODY()

public:
	ARANDVehicle();

	// --- IInteractable ------------------------------------------------------

	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(const AActor* Interactor) const override;

	// --- Ownership / theft --------------------------------------------------

	/** When false, entering this vehicle is treated as theft (adds SAPS heat). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	bool bOwned = false;

	/** SAPS heat applied to the driver on theft (entry while !bOwned). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float TheftHeat = 15.0f;

	/** True while André is currently driving this vehicle. */
	UFUNCTION(BlueprintPure, Category = "Vehicle")
	bool IsOccupied() const { return DriverCharacter != nullptr; }

protected:
	/** Heat layer for the vehicle itself (e.g. plates flagged after a chase). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wanted")
	TObjectPtr<UWantedComponent> WantedComponent;

	/** Local offset from the vehicle origin to the driver-door drop point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FVector DriverDoorOffset = FVector(0.0f, -120.0f, 0.0f);

	/** Exit (F) is bound here while the vehicle is possessed. */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	/** André while he is driving; null when the vehicle is empty. */
	UPROPERTY()
	TObjectPtr<ARANDCharacter> DriverCharacter;

	/** The controller that possessed André before he entered — restored on exit. */
	UPROPERTY()
	TObjectPtr<AController> SavedController;

	/** Mapping context + action that bind the exit key while driving. */
	UPROPERTY()
	TObjectPtr<class UInputMappingContext> VehicleMappingContext;

	UPROPERTY()
	TObjectPtr<class UInputAction> ExitAction;

	/** Drops André out of the vehicle at the driver door and restores control. */
	void ExitVehicle();
};
