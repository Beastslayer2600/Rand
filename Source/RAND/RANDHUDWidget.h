// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WantedComponent.h"
#include "RANDHUDWidget.generated.h"

class ARANDCharacter;
class UHealthComponent;
class UInteractionComponent;
class URANDTimeComponent;
class URANDMissionManager;
class URANDMinimapWidget;
class UProgressBar;
class UTextBlock;
class UVerticalBox;

/**
 * URANDHUDWidget — the in-game HUD, built as a UMG widget entirely in C++ so it
 * needs no editor-authored WBP asset (mirroring the project's code-first input
 * setup). Equivalent to a designer's "WBP_HUD" but constructed in RebuildWidget.
 *
 * Three elements:
 *   - Heat indicator: three rows (SAPS, Hawks, Rivals), each a name label, a
 *     bar filled to the agency's EHeatLevel bucket, and the bucket's name
 *     (None/Noticed/Wanted/Manhunt/Burned). Driven by OnHeatChanged.
 *   - Health bar: a bar with a numeric "cur/max" overlay, bound to
 *     UHealthComponent::OnHealthChanged.
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

	/** Shows/hides the unread-message indicator (driven by the phone widget). */
	void SetPhoneNotification(bool bHasUnread);

	/** Human-readable name for a heat bucket, used by the heat rows. */
	static FText HeatLevelToText(EHeatLevel Level);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

	// --- Widget tree (built in RebuildWidget) -------------------------------

	UPROPERTY()
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY()
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY()
	TObjectPtr<UTextBlock> InteractionText;

	UPROPERTY()
	TObjectPtr<UTextBlock> TimeText;

	/** Bottom-right active-mission panel (title + objective lines). */
	UPROPERTY()
	TObjectPtr<UVerticalBox> MissionBox;

	/** Top-center "new message" indicator. */
	UPROPERTY()
	TObjectPtr<UTextBlock> PhoneNotificationText;

	/** Top-right minimap. */
	UPROPERTY()
	TObjectPtr<URANDMinimapWidget> Minimap;

	/** Index order matches EAgency: 0 SAPS, 1 Hawks, 2 Rivals. */
	UPROPERTY()
	TArray<TObjectPtr<UProgressBar>> HeatBars;

	/** Per-agency level name ("None".."Burned"), index-aligned with HeatBars. */
	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> HeatLevelTexts;

private:
	static constexpr int32 AgencyCount = 3;
	static constexpr int32 MaxHeatLevel = 4; // EHeatLevel::Burned

	TWeakObjectPtr<UHealthComponent> BoundHealth;
	TWeakObjectPtr<UInteractionComponent> BoundInteraction;
	TWeakObjectPtr<UWantedComponent> BoundWanted;
	TWeakObjectPtr<URANDTimeComponent> BoundTime;
	TWeakObjectPtr<URANDMissionManager> BoundMissions;

	/** Builds one "Name [bar] Level" row and registers its widgets. */
	class UHorizontalBox* BuildHeatRow(int32 AgencyIdx, const TCHAR* DisplayName, const FLinearColor& Color);

	// --- Delegate handlers --------------------------------------------------

	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float Delta);

	UFUNCTION()
	void HandleHeatChanged(EAgency Agency, EHeatLevel OldLevel, EHeatLevel NewLevel);

	UFUNCTION()
	void HandleTargetChanged(AActor* NewTarget);

	UFUNCTION()
	void HandleMinutePassed(int32 Day, int32 Hour, int32 Minute);

	UFUNCTION()
	void HandleMissionStarted(FName MissionID);

	UFUNCTION()
	void HandleObjectiveComplete(FName MissionID, int32 ObjectiveIndex);

	UFUNCTION()
	void HandleMissionComplete(FName MissionID);

	UFUNCTION()
	void HandleMissionFailed(FName MissionID);

	/** Rebuilds the bottom-right mission panel from the active mission. */
	void UpdateMissionDisplay();

	/** Pushes current values into the widgets (used on initial bind). */
	void RefreshAll();
};
