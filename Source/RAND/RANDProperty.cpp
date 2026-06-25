// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDProperty.h"
#include "RANDCharacter.h"
#include "EconomyComponent.h"
#include "BusinessManager.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// In-game hours per month used to convert monthly rent to the business
	// manager's per-hour passive income (30 days x 24 hours).
	constexpr float GameHoursPerMonth = 30.0f * 24.0f;
}

ARANDProperty::ARANDProperty()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PropertyMesh"));
	SetRootComponent(Mesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
	// Building-ish proportions.
	Mesh->SetRelativeScale3D(FVector(3.0f, 3.0f, 4.0f));

	// Query-overlap so the player's interaction sphere detects the property.
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	Mesh->SetGenerateOverlapEvents(true);
}

FText ARANDProperty::GetInteractionPrompt_Implementation() const
{
	if (bIsOwned)
	{
		return NSLOCTEXT("RANDProperty", "ViewProperty", "View Property");
	}
	return FText::FromString(FString::Printf(TEXT("Buy Property R%.0f"), PurchasePrice));
}

bool ARANDProperty::CanInteract_Implementation(const AActor* /*Interactor*/) const
{
	// Always interactable: owned -> view, unowned -> attempt purchase.
	return true;
}

void ARANDProperty::OnInteract_Implementation(AActor* Interactor)
{
	if (bIsOwned)
	{
		return; // Already owned — "View Property" is a no-op for now.
	}

	ARANDCharacter* Buyer = Cast<ARANDCharacter>(Interactor);
	if (!Buyer)
	{
		return;
	}

	URANDEconomyComponent* Economy = Buyer->GetEconomyComponent();
	if (!Economy)
	{
		return;
	}

	// Charge the price; abort if André can't afford it.
	if (!Economy->DeductFunds(PurchasePrice, FString::Printf(TEXT("Property: %s"), *PropertyName)))
	{
		return;
	}

	bIsOwned = true;
	OwnerName = Buyer->GetName();

	// Register the property as a Legitimate-tier business so its rent accrues.
	if (URANDBusinessManager* Businesses = Buyer->GetBusinessManager())
	{
		FRANDBusiness Business;
		Business.BusinessName = PropertyName;
		Business.BusinessType = ERANDBusinessType::Legitimate;
		Business.PassiveIncomePerHour = MonthlyRentalIncome / GameHoursPerMonth;
		Business.HeatGenerationRate = 0.0f;
		Business.bIsActive = true;
		Businesses->AddBusiness(Business);
	}

	OnPurchased.Broadcast(Buyer);
}
