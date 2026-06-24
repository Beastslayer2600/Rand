// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IInteractable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * IInteractable — implemented by any actor André can interact with (NPCs,
 * pickups, doors, vehicles, etc.). The interaction component calls these when
 * the player presses E while the actor is the closest candidate in range.
 */
class RAND_API IInteractable
{
	GENERATED_BODY()

public:
	/** Returns the label shown in the interaction prompt, e.g. "Talk to Sipho". */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionPrompt() const;

	/** Called when the player confirms interaction. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteract(AActor* Interactor);

	/**
	 * Optional gate — return false to suppress the prompt and block interaction
	 * (e.g. a vendor that's only open at night, a door that needs a key card).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(const AActor* Interactor) const;
};
