// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IInteractable.h"
#include "RANDProperty.generated.h"

class UStaticMeshComponent;

/** Broad category of a buyable property. */
UENUM(BlueprintType)
enum class EPropertyType : uint8
{
	Residential	UMETA(DisplayName = "Residential"),
	Commercial	UMETA(DisplayName = "Commercial"),
	Industrial	UMETA(DisplayName = "Industrial"),
	Government	UMETA(DisplayName = "Government"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPropertyPurchased, AActor*, Buyer);

/**
 * ARANDProperty — a buyable building. Implements IInteractable: when unowned the
 * prompt reads "Buy Property R<price>"; once owned it reads "View Property".
 *
 * Purchase deducts the price from the buyer's economy and, on success, registers
 * the property as a Legitimate-tier FRANDBusiness in their business manager so
 * its rent flows as passive income.
 */
UCLASS()
class RAND_API ARANDProperty : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	ARANDProperty();

	// --- IInteractable ------------------------------------------------------

	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(const AActor* Interactor) const override;

	// --- Property data ------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	FString PropertyName = TEXT("Unnamed Property");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	EPropertyType PropertyType = EPropertyType::Residential;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float PurchasePrice = 250000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float MonthlyRentalIncome = 8000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	bool bIsOwned = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	FString OwnerName;

	/** Fired once when the property is bought. */
	UPROPERTY(BlueprintAssignable, Category = "Property")
	FOnPropertyPurchased OnPurchased;

private:
	UPROPERTY(VisibleAnywhere, Category = "Property", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;
};
