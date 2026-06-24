// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IInteractable.h"
#include "WantedComponent.h"      // EAgency
#include "RANDLanguageTypes.h"    // ELanguageGroup, EDistrict, ESituationType
#include "RANDCharacter_NPC.generated.h"

class URANDDialogueComponent;

/**
 * Broad behavioural archetype of an NPC. Drives the default agency a witness
 * reports to, and later gates dialogue/schedule/reaction logic.
 */
UENUM(BlueprintType)
enum class ENPCType : uint8
{
	Civilian	UMETA(DisplayName = "Civilian"),
	Official	UMETA(DisplayName = "Official"),
	Criminal	UMETA(DisplayName = "Criminal"),
	Police		UMETA(DisplayName = "Police"),
	Contact		UMETA(DisplayName = "Contact"),
};

/**
 * ARANDCharacter_NPC — base class for every non-player character.
 *
 * Implements IInteractable so André can talk to them ("Talk to <name>"),
 * carries a dialogue component for conversations, and can witness crimes:
 * ReportCrime adds heat to a nearby player's UWantedComponent. (The heat lives
 * on the player, not the NPC — the witness is just the source.)
 */
UCLASS()
class RAND_API ARANDCharacter_NPC : public ACharacter, public IInteractable
{
	GENERATED_BODY()

public:
	ARANDCharacter_NPC();

	// --- IInteractable ------------------------------------------------------

	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(const AActor* Interactor) const override;

	// --- Identity -----------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FString NPCName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	ENPCType NPCType = ENPCType::Civilian;

	/** Home language; selects which colloquial reaction lines this NPC uses. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	ELanguageGroup LanguageGroup = ELanguageGroup::English;

	// --- Language / reactions ----------------------------------------------

	/**
	 * Assigns this NPC a home language by weighted-random draw from a district's
	 * profile. Call at spawn time to place the NPC in its neighbourhood's mix.
	 */
	UFUNCTION(BlueprintCallable, Category = "NPC|Language")
	void AssignLanguageFromDistrict(EDistrict District);

	/** A random colloquial line for the given situation in this NPC's language. */
	UFUNCTION(BlueprintCallable, Category = "NPC|Language")
	FText GetReactionLine(ESituationType Situation) const;

	// --- Witnessing ---------------------------------------------------------

	/** Max distance (cm) at which this NPC can witness/report a crime. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Witness")
	float WitnessRadius = 1500.0f;

	/**
	 * Report a witnessed crime: if the player is within WitnessRadius, adds
	 * HeatAmount to their UWantedComponent for the given agency. Returns true
	 * if heat was applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "NPC|Witness")
	bool ReportCrime(EAgency Agency, float HeatAmount);

	/** Reports using the agency implied by this NPC's type (see GetDefaultReportingAgency). */
	UFUNCTION(BlueprintCallable, Category = "NPC|Witness")
	bool WitnessCrime(float HeatAmount);

	/** The agency this NPC type naturally reports to. */
	UFUNCTION(BlueprintPure, Category = "NPC|Witness")
	EAgency GetDefaultReportingAgency() const;

	// --- Accessors ----------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "NPC")
	URANDDialogueComponent* GetDialogueComponent() const { return DialogueComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URANDDialogueComponent> DialogueComponent;
};
