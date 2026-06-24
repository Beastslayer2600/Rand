// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WantedComponent.h"
#include "RANDHUDWidget.generated.h"

class ARANDCharacter;
class UHealthComponent;
class UInteractionComponent;
class UProgressBar;
class UTextBlock;

/**
 * URANDHUDWidget — the in-game HUD, built as a UMG widget entirely in C++ so it
 * needs no editor-authored WBP asset (mirroring the project's code-first input
 * setup). Equivalent to a designer's "WBP_HUD" but constructed in RebuildWidget.
 *
 * Three elements:
 *   - Heat indicator: three bars (SAPS, Hawks, Rivals), each filled to the
 *     agency's current EHeatLevel bucket (None..Burned).
 *   - Health bar: bound to UHealthComponent::OnHealthChanged.
 *   - Interaction prompt: shows the active IInteractable's prompt while
 *     UInteractionComponent has a CurrentTarget; hidden when there is none.
 */
UCLASS()
class RAND_API URANDHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	URANDHUDWidget(const FObjectInitializer& ObjectInitializer);

	/** Subscribes the HUD to a character's health/heat/interaction systems. */
	void BindToCharacter(ARANDCharacter* Character);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

	// --- Widget tree (built in RebuildWidget) -------------------------------

	UPROPERTY()
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY()
	TObjectPtr<UTextBlock> InteractionText;

	/** Index order matches EAgency: 0 SAPS, 1 Hawks, 2 Rivals. */
	UPROPERTY()
	TArray<TObjectPtr<UProgressBar>> HeatBars;

private:
	static constexpr int32 AgencyCount = 3;
	static constexpr int32 MaxHeatLevel = 4; // EHeatLevel::Burned

	TWeakObjectPtr<UHealthComponent> BoundHealth;
	TWeakObjectPtr<UInteractionComponent> BoundInteraction;
	TWeakObjectPtr<UWantedComponent> BoundWanted;

	// --- Delegate handlers --------------------------------------------------

	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float Delta);

	UFUNCTION()
	void HandleHeatChanged(EAgency Agency, EHeatLevel OldLevel, EHeatLevel NewLevel);

	UFUNCTION()
	void HandleTargetChanged(AActor* NewTarget);

	/** Pushes current values into the widgets (used on initial bind). */
	void RefreshAll();
};
