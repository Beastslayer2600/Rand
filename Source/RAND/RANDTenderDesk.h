// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IInteractable.h"
#include "RANDTenderDesk.generated.h"

class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTenderSubmitted, AActor*, Interactor);

/**
 * ARANDTenderDesk — an interactable office desk used by Mission 1 to "submit
 * the tender". Implements IInteractable ("Submit Tender"); the first interaction
 * fires OnTenderSubmitted and then stops offering.
 */
UCLASS()
class RAND_API ARANDTenderDesk : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	ARANDTenderDesk();

	// --- IInteractable ------------------------------------------------------

	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(const AActor* Interactor) const override;

	/** Fired once, when the tender is submitted. */
	UPROPERTY(BlueprintAssignable, Category = "Tender")
	FOnTenderSubmitted OnTenderSubmitted;

	/** Gate: when false the desk is inert (e.g. before the player has met Thandi). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tender")
	bool bAcceptingSubmission = true;

	UPROPERTY(BlueprintReadOnly, Category = "Tender")
	bool bSubmitted = false;

private:
	UPROPERTY(VisibleAnywhere, Category = "Tender", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;
};
