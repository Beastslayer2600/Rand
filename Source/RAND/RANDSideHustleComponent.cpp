// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDSideHustleComponent.h"
#include "RANDGameMode.h"
#include "RANDCharacter.h"
#include "EconomyComponent.h"
#include "WantedComponent.h"
#include "TimeComponent.h"
#include "RANDPhoneWidget.h"
#include "RANDSARSComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

URANDSideHustleComponent::URANDSideHustleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

URANDSideHustleComponent* URANDSideHustleComponent::Get(const UObject* WorldContext)
{
	if (UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr)
	{
		if (ARANDGameMode* GM = World->GetAuthGameMode<ARANDGameMode>())
		{
			return GM->GetSideHustle();
		}
	}
	return nullptr;
}

void URANDSideHustleComponent::BeginPlay()
{
	Super::BeginPlay();
	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		Clock->OnHourPassed.AddDynamic(this, &URANDSideHustleComponent::HandleHourPassed);
	}
	BindPhone();
}

void URANDSideHustleComponent::BindPhone()
{
	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
	{
		Phone->OnMessageOptionSelected.AddDynamic(this, &URANDSideHustleComponent::HandlePhoneOption);
	}
	else if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(BindTimer, this, &URANDSideHustleComponent::BindPhone, 0.5f, false);
	}
}

void URANDSideHustleComponent::HandleHourPassed(int32, int32)
{
	if (bOfferOpen) return;
	++HoursSinceOffer;
	if (HoursSinceOffer >= 4) OfferJob();
}

void URANDSideHustleComponent::OfferJob()
{
	URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this);
	if (!Phone) return;

	HoursSinceOffer = 0;
	bOfferOpen = true;

	struct FJob { const TCHAR* Sender; const TCHAR* Body; const TCHAR* Accept; FName Id; };
	static const FJob Jobs[] =
	{
		{ TEXT("Unknown number"), TEXT("Envelope at Park Station. R12k. No questions."), TEXT("Take the envelope"), TEXT("Hustle_Envelope") },
		{ TEXT("Sipho Dlamini"), TEXT("A contractor needs a facilitation letter. R8k. Clean."), TEXT("Write the letter"), TEXT("Hustle_Letter") },
		{ TEXT("Bra Mike"), TEXT("Package on the Pretoria run. R18k. Don't look inside."), TEXT("Move the package"), TEXT("Hustle_Package") },
		{ TEXT("Unknown number"), TEXT("Someone at the committee is talking. Make them quiet. R25k."), TEXT("Handle it"), TEXT("Hustle_Witness") },
	};

	const FJob& Job = Jobs[FMath::RandRange(0, 3)];
	PendingAccept = Job.Id;

	TArray<FRANDMessageOption> Options;
	FRANDMessageOption A; A.Label = FText::FromString(Job.Accept); A.ActionId = Job.Id; Options.Add(A);
	FRANDMessageOption B; B.Label = FText::FromString(TEXT("Ignore")); B.ActionId = TEXT("Hustle_Ignore"); Options.Add(B);
	Phone->ReceiveMessageWithOptions(Job.Sender, FText::FromString(Job.Body), Options);
}

void URANDSideHustleComponent::HandlePhoneOption(FName ActionId)
{
	if (ActionId.ToString().StartsWith(TEXT("Hustle_"))) Resolve(ActionId);
}

void URANDSideHustleComponent::Resolve(FName ActionId)
{
	bOfferOpen = false;
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player) return;

	float Payout = 0.f;
	float Heat = 0.f;
	EAgency Agency = EAgency::SAPS;
	FString Source;

	if (ActionId == TEXT("Hustle_Envelope")) { Payout = 12000.f; Heat = 6.f; Agency = EAgency::Hawks; Source = TEXT("Envelope job"); }
	else if (ActionId == TEXT("Hustle_Letter")) { Payout = 8000.f; Source = TEXT("Facilitation letter"); }
	else if (ActionId == TEXT("Hustle_Package")) { Payout = 18000.f; Heat = 8.f; Agency = EAgency::Rivals; Source = TEXT("Pretoria package"); }
	else if (ActionId == TEXT("Hustle_Witness")) { Payout = 25000.f; Heat = 16.f; Agency = EAgency::SAPS; Source = TEXT("Committee silence"); }
	else return;

	if (URANDSARSComponent* SARS = URANDSARSComponent::Get(this)) Payout = SARS->Adjust(Payout);
	if (URANDEconomyComponent* Econ = Player->GetEconomyComponent()) Econ->AddFunds(Payout, Source);
	if (Heat > 0.f && Player->GetWantedComponent()) Player->GetWantedComponent()->AddHeat(Agency, Heat);
}
