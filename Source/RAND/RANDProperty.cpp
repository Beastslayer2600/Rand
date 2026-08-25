// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDProperty.h"
#include "RANDCharacter.h"
#include "EconomyComponent.h"
#include "BusinessManager.h"
#include "HealthComponent.h"
#include "WantedComponent.h"
#include "RANDSaveGameManager.h"
#include "RANDGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

namespace { constexpr float GameHoursPerMonth = 30.0f * 24.0f; }

ARANDProperty::ARANDProperty()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PropertyMesh"));
	SetRootComponent(Mesh);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded()) Mesh->SetStaticMesh(CubeMesh.Object);
	Mesh->SetRelativeScale3D(FVector(3.0f, 3.0f, 4.0f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	Mesh->SetGenerateOverlapEvents(true);
}

FText ARANDProperty::GetInteractionPrompt_Implementation() const
{
	if (bIsOwned) return NSLOCTEXT("RANDProperty", "Rest", "Rest — heal, cool heat, save");
	return FText::FromString(FString::Printf(TEXT("Buy Property R%.0f"), PurchasePrice));
}

bool ARANDProperty::CanInteract_Implementation(const AActor*) const { return true; }

void ARANDProperty::OnInteract_Implementation(AActor* Interactor)
{
	ARANDCharacter* Buyer = Cast<ARANDCharacter>(Interactor);
	if (!Buyer) return;

	if (bIsOwned)
	{
		if (UHealthComponent* Health = Buyer->GetHealthComponent()) Health->Revive();
		if (UWantedComponent* Wanted = Buyer->GetWantedComponent())
		{
			Wanted->SetHeat(EAgency::SAPS, FMath::Max(0.f, Wanted->GetRawHeat(EAgency::SAPS) - 20.f));
		}
		if (ARANDGameMode* GM = Cast<ARANDGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			if (URANDSaveGameManager* Save = GM->GetSaveManager()) Save->SaveGame();
		}
		return;
	}

	URANDEconomyComponent* Economy = Buyer->GetEconomyComponent();
	if (!Economy) return;
	if (!Economy->DeductFunds(PurchasePrice, FString::Printf(TEXT("Property: %s"), *PropertyName))) return;

	bIsOwned = true;
	OwnerName = Buyer->GetName();
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
