// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionTargetChanged, AActor*, NewTarget);

/**
 * UInteractionComponent — attaches to ARANDCharacter and manages proximity-
 * based interaction. A sphere overlap finds nearby IInteractable actors; the
 * closest one that passes CanInteract becomes the active target and drives the
 * HUD prompt. E-key calls OnInteract on that target.
 *
 * The component owns the E key binding by adding IA_Interact to the character's
 * mapping context in BeginPlay, keeping the character class tidy.
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	/** Radius (cm) within which IInteractable actors are considered. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionRadius = 200.0f;

	/** Currently highlighted interactable, or nullptr. */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

	/** Prompt text of the current target, or empty if none. */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetPromptText() const;

	/** Broadcast whenever the best interactable target changes (including to nullptr). */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractionTargetChanged OnTargetChanged;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** Sphere used to gather overlapping interactable actors each tick. */
	UPROPERTY()
	TObjectPtr<USphereComponent> ProximitySphere;

	/** The actor currently selected as the best interact candidate. */
	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentTarget;

	/** Performs the interaction with the current target. Bound to E key. */
	void TryInteract();

	/** Scans overlapping actors and returns the nearest valid IInteractable. */
	AActor* FindBestTarget() const;
};
