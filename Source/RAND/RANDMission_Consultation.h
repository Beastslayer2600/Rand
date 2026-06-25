// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RANDMission_Consultation.generated.h"

class USphereComponent;
class ARANDCharacter_NPC;
class ARANDTenderDesk;
class UPrimitiveComponent;

/**
 * ARANDMission_Consultation — Mission 1, "The Consultation" (GDD).
 *
 * Place this actor at the Marshalltown office. On BeginPlay it registers and
 * starts the mission, spawns Thandi (an Official NPC speaking Afrikaans) and a
 * tender desk, and drives four objectives:
 *   0  Drive to the Marshalltown office  (office trigger overlap)
 *   1  Speak to Thandi                   (her dialogue ends)
 *   2  Submit the tender                 (desk interaction)
 *   3  Respond to Thandi's offer         (AcceptBribe / DeclineBribe)
 *
 * Accepting pays R85,000 and adds 20 Hawks heat, and records bAcceptedBribe on
 * the player. The bribe offer itself is delivered via the phone (see the phone
 * system), which calls AcceptBribe/DeclineBribe.
 */
UCLASS()
class RAND_API ARANDMission_Consultation : public AActor
{
	GENERATED_BODY()

public:
	ARANDMission_Consultation();

	/** Stable id this mission registers under. */
	static const FName MissionID;

	/** Player takes the bribe: +R85,000, +20 Hawks heat, sets bAcceptedBribe, completes the mission. */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void AcceptBribe();

	/** Player turns it down: clean completion. */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void DeclineBribe();

	/** Presents the bribe choice after the tender is submitted (hook for the phone). */
	UFUNCTION(BlueprintCallable, Category = "Mission")
	void OfferBribe();

	/** True once the tender is in and the offer is on the table. */
	UFUNCTION(BlueprintPure, Category = "Mission")
	bool IsBribeOffered() const { return bBribeOffered; }

protected:
	virtual void BeginPlay() override;

	/** Local offsets from this actor for the spawned props. */
	UPROPERTY(EditAnywhere, Category = "Mission")
	FVector ThandiSpawnOffset = FVector(250.0f, 0.0f, 90.0f);

	UPROPERTY(EditAnywhere, Category = "Mission")
	FVector DeskSpawnOffset = FVector(0.0f, 250.0f, 40.0f);

	/** Radius of the office-arrival trigger. */
	UPROPERTY(EditAnywhere, Category = "Mission")
	float OfficeTriggerRadius = 400.0f;

	UPROPERTY(EditAnywhere, Category = "Mission")
	TSubclassOf<ARANDCharacter_NPC> ThandiClass;

	UPROPERTY(EditAnywhere, Category = "Mission")
	TSubclassOf<ARANDTenderDesk> DeskClass;

private:
	UPROPERTY(VisibleAnywhere, Category = "Mission", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> OfficeTrigger;

	UPROPERTY()
	TObjectPtr<ARANDCharacter_NPC> Thandi;

	UPROPERTY()
	TObjectPtr<ARANDTenderDesk> Desk;

	// One-shot guards so each objective fires once.
	bool bReachedOffice = false;
	bool bTalkedToThandi = false;
	bool bSubmittedTender = false;
	bool bBribeOffered = false;
	bool bResolved = false;

	UFUNCTION()
	void HandleOfficeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void HandleThandiDialogueEnded();

	UFUNCTION()
	void HandleTenderSubmitted(AActor* Interactor);

	/** Routes the player's phone reply (accept/decline) back to the mission. */
	UFUNCTION()
	void HandlePhoneOption(FName ActionId);

	/** Authors Thandi's short Afrikaans intro conversation. */
	void SetupThandiDialogue();

	/** Sends the opening mission briefing as a phone message (deferred). */
	void SendBriefing();

	FTimerHandle BriefingTimer;
};
