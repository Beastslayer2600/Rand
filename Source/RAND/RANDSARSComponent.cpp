// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDSARSComponent.h"
#include "RANDGameMode.h"
#include "RANDCharacter.h"
#include "EconomyComponent.h"
#include "WantedComponent.h"
#include "TimeComponent.h"
#include "RANDPhoneWidget.h"
#include "Kismet/GameplayStatics.h"

URANDSARSComponent::URANDSARSComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

URANDSARSComponent* URANDSARSComponent::Get(const UObject* WorldContext)
{
	if (!WorldContext)
	{
		return nullptr;
	}
	if (UWorld* World = WorldContext->GetWorld())
	{
		if (ARANDGameMode* GM = World->GetAuthGameMode<ARANDGameMode>())
		{
			return GM->GetSARSComponent();
		}
	}
	return nullptr;
}

void URANDSARSComponent::BeginPlay()
{
	Super::BeginPlay();

	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		Clock->OnHourPassed.AddDynamic(this, &URANDSARSComponent::HandleHourPassed);
		Clock->OnDayPassed.AddDynamic(this, &URANDSARSComponent::HandleDayPassed);
	}
}

bool URANDSARSComponent::FileReturn()
{
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player)
	{
		return false;
	}
	URANDEconomyComponent* Econ = Player->GetEconomyComponent();
	if (!Econ)
	{
		return false;
	}

	const float Bill = FMath::Max(1000.0f, Econ->GetBalance() * FilingRate);
	if (!Econ->DeductFunds(Bill, TEXT("SARS - provisional return")))
	{
		return false;
	}

	bInvestigationActive = false;
	bFiledThisWeek = true;
	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		LastFiledDay = Clock->GetCurrentDay();
	}

	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
	{
		Phone->ReceiveMessage(TEXT("SARS eFiling"),
			FText::FromString(FString::Printf(TEXT("Return received. Assessment: R%.0f"), Bill)));
	}
	return true;
}

void URANDSARSComponent::HandleHourPassed(int32 /*Day*/, int32 /*Hour*/)
{
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player)
	{
		return;
	}

	URANDEconomyComponent* Econ = Player->GetEconomyComponent();
	if (!Econ)
	{
		return;
	}

	if (Econ->GetBalance() >= FilingThresholdZAR && !bFiledThisWeek)
	{
		if (!bInvestigationActive)
		{
			bInvestigationActive = true;
			if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
			{
				Phone->ReceiveMessage(TEXT("SARS eFiling"),
					NSLOCTEXT("RANDSARS", "Notice",
						"Notice of audit. File a provisional return (F7) or we escalate to the Hawks."));
			}
		}

		if (UWantedComponent* Wanted = Player->GetWantedComponent())
		{
			Wanted->AddHeat(EAgency::Hawks, InvestigationHeatPerHour);
		}
	}
}

void URANDSARSComponent::HandleDayPassed(int32 Day)
{
	InflationMultiplier += DailyInflationStep;
	OnInflationChanged.Broadcast(InflationMultiplier);

	if (LastFiledDay > 0 && (Day - LastFiledDay) >= 7)
	{
		bFiledThisWeek = false;
	}
}

void URANDSARSComponent::LoadState(float InMultiplier, bool bInvestigating)
{
	InflationMultiplier = FMath::Max(1.0f, InMultiplier);
	bInvestigationActive = bInvestigating;
}
