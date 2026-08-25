// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDCareerComponent.h"
#include "RANDCharacter.h"
#include "EconomyComponent.h"
#include "BusinessManager.h"
#include "WantedComponent.h"
#include "TimeComponent.h"
#include "RANDPhoneWidget.h"

URANDCareerComponent::URANDCareerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDCareerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		Clock->OnHourPassed.AddDynamic(this, &URANDCareerComponent::HandleHourPassed);
	}
}

void URANDCareerComponent::SetStage(ERANDCareerStage NewStage)
{
	if (NewStage == Stage)
	{
		return;
	}
	const ERANDCareerStage Old = Stage;
	Stage = NewStage;
	OnStageChanged.Broadcast(Old, Stage);
}

void URANDCareerComponent::ResolveEnding(ERANDEnding InEnding)
{
	if (Ending != ERANDEnding::None || InEnding == ERANDEnding::None)
	{
		return;
	}
	Ending = InEnding;
	OnCampaignEnded.Broadcast(Ending);
}

bool URANDCareerComponent::GoDark()
{
	if (bIsDark)
	{
		return false;
	}

	ARANDCharacter* Player = Cast<ARANDCharacter>(GetOwner());
	if (!Player)
	{
		return false;
	}

	URANDEconomyComponent* Econ = Player->GetEconomyComponent();
	if (!Econ || !Econ->DeductFunds(DarkCostZAR, TEXT("Go dark - burner + safe house")))
	{
		return false;
	}

	if (UWantedComponent* Wanted = Player->GetWantedComponent())
	{
		Wanted->ClearHeat(EAgency::SAPS);
	}

	if (URANDBusinessManager* Biz = Player->GetBusinessManager())
	{
		TArray<FRANDBusiness> Copy = Biz->GetBusinesses();
		for (FRANDBusiness& B : Copy)
		{
			if (B.BusinessType == ERANDBusinessType::Black)
			{
				B.bIsActive = false;
			}
		}
		Biz->SetBusinesses(Copy);
	}

	bIsDark = true;
	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		DarkUntilDay = Clock->GetCurrentDay();
		DarkUntilHour = Clock->GetCurrentHour() + DarkDurationHours;
		while (DarkUntilHour >= 24)
		{
			DarkUntilHour -= 24;
			++DarkUntilDay;
		}
	}

	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
	{
		Phone->ReceiveMessage(TEXT("Unknown"),
			NSLOCTEXT("RANDCareer", "DarkOn", "Number burned. Sit tight. Don't lift a finger until the heat dies."));
	}
	return true;
}

void URANDCareerComponent::ComeUpForAir()
{
	if (!bIsDark)
	{
		return;
	}
	bIsDark = false;

	if (ARANDCharacter* Player = Cast<ARANDCharacter>(GetOwner()))
	{
		if (URANDBusinessManager* Biz = Player->GetBusinessManager())
		{
			TArray<FRANDBusiness> Copy = Biz->GetBusinesses();
			for (FRANDBusiness& B : Copy)
			{
				B.bIsActive = true;
			}
			Biz->SetBusinesses(Copy);
		}
	}

	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
	{
		Phone->ReceiveMessage(TEXT("Unknown"),
			NSLOCTEXT("RANDCareer", "DarkOff", "You're back. Don't waste it."));
	}
}

void URANDCareerComponent::HandleHourPassed(int32 Day, int32 Hour)
{
	if (!bIsDark)
	{
		return;
	}
	if (Day > DarkUntilDay || (Day == DarkUntilDay && Hour >= DarkUntilHour))
	{
		ComeUpForAir();
	}
}

void URANDCareerComponent::LoadState(ERANDCareerStage InStage, ERANDEnding InEnding)
{
	Stage = InStage;
	Ending = InEnding;
}
