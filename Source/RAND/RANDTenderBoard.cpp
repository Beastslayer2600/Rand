// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDTenderBoard.h"
#include "RANDGameMode.h"
#include "RANDCharacter.h"
#include "EconomyComponent.h"
#include "WantedComponent.h"
#include "TimeComponent.h"
#include "RANDReputationComponent.h"
#include "RANDPhoneWidget.h"
#include "RANDSARSComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	const FName CleanId(TEXT("Tender_Clean"));
	const FName RigId(TEXT("Tender_Rig"));
	const FName PassId(TEXT("Tender_Pass"));
	const FName FrontBuyId(TEXT("Tender_FrontBuy"));
	const FName FrontSkipId(TEXT("Tender_FrontSkip"));

	const TCHAR* const PortalSender = TEXT("eTender Portal");
	const TCHAR* const SCMSender = TEXT("Thandi Mokoena");

	/** Tenders nobody answers close at the end of the business day. */
	constexpr int32 TenderExpiryHours = 8;

	/** Day the fronting offer first comes in. */
	constexpr int32 FrontOfferDay = 3;

	const TCHAR* const TenderNames[] =
	{
		TEXT("Tshwane: pothole repair, Mamelodi"),
		TEXT("CoJ: street-light replacement, Braamfontein"),
		TEXT("Gauteng Health: clinic refurbishment, Soweto"),
		TEXT("DPWR: security services, Union Buildings annex"),
		TEXT("Joburg Water: pipe replacement, Hillbrow"),
		TEXT("Tshwane: fleet maintenance, Pretoria West depot"),
		TEXT("Gauteng Education: school toilets, Atteridgeville"),
		TEXT("PRASA: fencing, Park Station yard"),
		TEXT("CoJ: PPE supply, emergency services"),
		TEXT("SANRAL: N1 offramp resurfacing, Samrand"),
	};
}

URANDTenderBoard::URANDTenderBoard()
{
	PrimaryComponentTick.bCanEverTick = false;
}

URANDTenderBoard* URANDTenderBoard::Get(const UObject* WorldContext)
{
	UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (!World) return nullptr;
	if (ARANDGameMode* GM = World->GetAuthGameMode<ARANDGameMode>())
	{
		return GM->GetTenderBoard();
	}
	return nullptr;
}

void URANDTenderBoard::BeginPlay()
{
	Super::BeginPlay();
	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		Clock->OnHourPassed.AddDynamic(this, &URANDTenderBoard::HandleHourPassed);
		Clock->OnDayPassed.AddDynamic(this, &URANDTenderBoard::HandleDayPassed);
	}
	BindPhone();
}

void URANDTenderBoard::BindPhone()
{
	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
	{
		Phone->OnMessageOptionSelected.AddDynamic(this, &URANDTenderBoard::HandlePhoneOption);
	}
	else if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(BindTimer, this, &URANDTenderBoard::BindPhone, 0.5f, false);
	}
}

float URANDTenderBoard::GradeLimit(int32 Grade)
{
	// CIDB tender value limits per grade (ZAR). Grade 9 is unlimited.
	static const float Limits[] = { 200000.f, 650000.f, 2000000.f, 4000000.f, 6500000.f,
		13000000.f, 40000000.f, 130000000.f, TNumericLimits<float>::Max() };
	return Limits[FMath::Clamp(Grade, 1, 9) - 1];
}

int32 URANDTenderBoard::PreferencePoints(int32 Level)
{
	// 80/20 preference point system: B-BBEE level 1..8, anything else non-compliant.
	static const int32 Points[] = { 0, 20, 18, 14, 12, 8, 6, 4, 2 };
	return (Level >= 1 && Level <= 8) ? Points[Level] : 0;
}

void URANDTenderBoard::HandleHourPassed(int32 Day, int32 Hour)
{
	if (bTenderOpen && ++HoursTenderOpen >= TenderExpiryHours)
	{
		bTenderOpen = false;
		if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this)) Phone->WithdrawChoice(PassId);
		Send(PortalSender, FString::Printf(TEXT("Closed: %s. No bid received from Venter Consulting."), *TenderName));
	}

	// The morning tender goes out at PostingHour; if the phone is busy, keep trying until close of business.
	if (!bPostedToday && !bTenderOpen && Hour >= PostingHour && Hour < PostingHour + TenderExpiryHours)
	{
		PostTender();
	}

	if (bFrontOfferOpen && ++HoursFrontOfferOpen >= TenderExpiryHours)
	{
		bFrontOfferOpen = false;
		bFrontRetryTomorrow = true;
		if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this)) Phone->WithdrawChoice(FrontBuyId);
	}

	if (!bFrontOffered && !bHasFront && Day >= FrontOfferDay && !bFrontOfferOpen && !bTenderOpen)
	{
		OfferFront();
	}
}

void URANDTenderBoard::HandleDayPassed(int32 /*Day*/)
{
	bPostedToday = false;
	if (bFrontRetryTomorrow && !bHasFront)
	{
		bFrontOffered = false;
	}
	bFrontRetryTomorrow = false;

	if (BlacklistDays > 0 && --BlacklistDays == 0)
	{
		Send(TEXT("National Treasury"), TEXT("Venter Consulting & Procurement has been removed from the Register for Tender Defaulters."));
	}

	// Whistle-blowers are a slow burn: risk decays, but every day it is rolled.
	if (WhistleblowerRisk > 0.f && FMath::FRand() < WhistleblowerRisk * 0.25f)
	{
		Expose();
	}
	WhistleblowerRisk = FMath::Max(0.f, WhistleblowerRisk - 0.02f);
}

void URANDTenderBoard::PostTender()
{
	URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this);
	if (!Phone || Phone->HasPendingChoice()) return;

	// Blacklisted without a front: the portal won't take the company's bids.
	if (IsBlacklisted() && !bHasFront)
	{
		bPostedToday = true;
		return;
	}

	// Values straddle the current grade limit so some tenders are only reachable by rigging.
	const float Ceiling = CIDBGrade >= 9 ? 200000000.f : GradeLimit(CIDBGrade);
	float Value = Ceiling * FMath::FRandRange(0.3f, 1.6f);
	if (URANDSARSComponent* SARS = URANDSARSComponent::Get(this)) Value = SARS->Adjust(Value);
	TenderValue = FMath::RoundToFloat(Value / 1000.f) * 1000.f;
	TenderName = TenderNames[FMath::RandRange(0, static_cast<int32>(UE_ARRAY_COUNT(TenderNames)) - 1)];

	bTenderOpen = true;
	bPostedToday = true;
	HoursTenderOpen = 0;

	const bool bOverGrade = TenderValue > GradeLimit(CIDBGrade);
	const float Bribe = FMath::Max(25000.f, TenderValue * BribeRate);

	TArray<FRANDMessageOption> Options;
	FRANDMessageOption Clean;
	Clean.Label = bOverGrade
		? FText::FromString(FString::Printf(TEXT("Bid clean — needs CIDB grade above %d"), CIDBGrade))
		: FText::FromString(FString::Printf(TEXT("Bid clean (BEE level %d, %d points)"), GetBEELevel(), PreferencePoints(GetBEELevel())));
	Clean.ActionId = CleanId;
	Options.Add(Clean);
	FRANDMessageOption Rig;
	Rig.Label = FText::FromString(FString::Printf(TEXT("Pay the SCM official — R%.0f"), Bribe));
	Rig.ActionId = RigId;
	Options.Add(Rig);
	FRANDMessageOption Pass;
	Pass.Label = FText::FromString(TEXT("Pass"));
	Pass.ActionId = PassId;
	Options.Add(Pass);

	Phone->ReceiveMessageWithOptions(PortalSender,
		FText::FromString(FString::Printf(TEXT("New tender: %s. Estimated value R%.0f. Closes in %d hours."), *TenderName, TenderValue, TenderExpiryHours)),
		Options);
}

void URANDTenderBoard::HandlePhoneOption(FName ActionId)
{
	if (bTenderOpen)
	{
		if (ActionId == CleanId) { BidClean(); return; }
		if (ActionId == RigId)   { RigTender(); return; }
		if (ActionId == PassId)  { bTenderOpen = false; return; }
	}

	if (bFrontOfferOpen && (ActionId == FrontBuyId || ActionId == FrontSkipId))
	{
		bFrontOfferOpen = false;
		if (ActionId == FrontSkipId)
		{
			Send(TEXT("Cousin Lerato"), TEXT("Your loss, bru. Everybody else has a cousin."));
			return;
		}
		ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
		URANDEconomyComponent* Econ = Player ? Player->GetEconomyComponent() : nullptr;
		URANDSARSComponent* SARS = URANDSARSComponent::Get(this);
		const float Cost = SARS ? SARS->Adjust(FrontCost) : FrontCost;
		if (Econ && Econ->DeductFunds(Cost, TEXT("BEE front: Lerato Holdings (51%)")))
		{
			bHasFront = true;
			Send(TEXT("Cousin Lerato"), TEXT("Lerato Holdings is 51% mine on paper. Level 1. I sign what you send."));
		}
		else
		{
			bFrontRetryTomorrow = true; // Offer again once the money might be there.
			Send(TEXT("Cousin Lerato"), TEXT("Paper costs money, bru. Call me when you have it."));
		}
	}
}

void URANDTenderBoard::BidClean()
{
	bTenderOpen = false;
	if (TenderValue > GradeLimit(CIDBGrade))
	{
		Send(PortalSender, FString::Printf(TEXT("Bid disqualified: CIDB grade %d does not cover R%.0f."), CIDBGrade, TenderValue));
		return;
	}

	// 80 points on price is roughly a coin-flip against the field; the 20 preference points tip it.
	const float WinChance = 0.2f + 0.5f * (PreferencePoints(GetBEELevel()) / 20.f);
	if (FMath::FRand() < WinChance)
	{
		Award();
	}
	else
	{
		Send(PortalSender, FString::Printf(TEXT("Unsuccessful: %s. Awarded to a bidder with higher preference points."), *TenderName));
	}
}

void URANDTenderBoard::RigTender()
{
	bTenderOpen = false;
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	URANDEconomyComponent* Econ = Player ? Player->GetEconomyComponent() : nullptr;
	if (!Econ) return;

	const float Bribe = FMath::Max(25000.f, TenderValue * BribeRate);
	if (!Econ->DeductFunds(Bribe, FString::Printf(TEXT("Bribe: SCM, %s"), *TenderName)))
	{
		Send(SCMSender, TEXT("No cash, no spec. Don't waste my airtime."));
		return;
	}

	const bool bOverGrade = TenderValue > GradeLimit(CIDBGrade);
	WhistleblowerRisk = FMath::Min(1.f, WhistleblowerRisk + (bOverGrade ? 0.25f : 0.15f));
	if (UWantedComponent* Wanted = Player->GetWantedComponent()) Wanted->AddHeat(EAgency::Hawks, 10.f);
	if (URANDReputationComponent* Rep = Player->GetReputationComponent()) Rep->AddStanding(ERANDContact::Thandi, 5.f);

	Send(SCMSender, bOverGrade
		? TEXT("Your grade is too small, so a graded company holds the paper and subcontracts it all to you. People notice these things.")
		: TEXT("The spec is written around you. The committee reads what I give them."));
	Award();
}

void URANDTenderBoard::Award()
{
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (URANDEconomyComponent* Econ = Player ? Player->GetEconomyComponent() : nullptr)
	{
		Econ->AddFunds(TenderValue * WinMargin, FString::Printf(TEXT("Tender: %s"), *TenderName));
	}

	// Every award through the front is one more set of papers someone can read.
	if (bHasFront) WhistleblowerRisk = FMath::Min(1.f, WhistleblowerRisk + 0.05f);

	++ContractsWon;
	Send(PortalSender, FString::Printf(TEXT("Awarded: %s to Venter Consulting & Procurement. R%.0f."), *TenderName, TenderValue));
	OnTenderAwarded.Broadcast(TenderName, TenderValue);

	// Delivered contracts build the track record CIDB grades on. Rigged ones count too -- on paper.
	if (CIDBGrade < 9 && ContractsWon % FMath::Max(1, ContractsPerGrade) == 0)
	{
		++CIDBGrade;
		Send(TEXT("CIDB"), CIDBGrade >= 9
			? FString(TEXT("Registration upgraded: Grade 9 GB. No tender value limit."))
			: FString::Printf(TEXT("Registration upgraded: Grade %d GB. Tender value limit R%.0f."), CIDBGrade, GradeLimit(CIDBGrade)));
	}
}

void URANDTenderBoard::OfferFront()
{
	URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this);
	if (!Phone || Phone->HasPendingChoice()) return;

	bFrontOffered = true;
	bFrontOfferOpen = true;
	HoursFrontOfferOpen = 0;
	URANDSARSComponent* SARS = URANDSARSComponent::Get(this);
	const float Cost = SARS ? SARS->Adjust(FrontCost) : FrontCost;

	TArray<FRANDMessageOption> Options;
	FRANDMessageOption Buy; Buy.Label = FText::FromString(FString::Printf(TEXT("Register the front — R%.0f"), Cost)); Buy.ActionId = FrontBuyId; Options.Add(Buy);
	FRANDMessageOption Skip; Skip.Label = FText::FromString(TEXT("Stay as you are")); Skip.ActionId = FrontSkipId; Options.Add(Skip);
	Phone->ReceiveMessageWithOptions(TEXT("Cousin Lerato"),
		FText::FromString(TEXT("Your BEE level is killing your bids. Put 51% in my name, I sign, you run it. Level 1 overnight. Treasury can't blacklist a company that isn't yours.")),
		Options);
}

void URANDTenderBoard::Expose()
{
	WhistleblowerRisk = 0.f;
	BlacklistDays = BlacklistLengthDays;
	OnBlacklisted.Broadcast(BlacklistDays);

	if (ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UWantedComponent* Wanted = Player->GetWantedComponent()) Wanted->AddHeat(EAgency::Hawks, 30.f);
		if (URANDReputationComponent* Rep = Player->GetReputationComponent()) Rep->AddStanding(ERANDContact::Thandi, -15.f);
	}

	Send(TEXT("News 24"), TEXT("EXCLUSIVE: Committee clerk hands Hawks a folder of rigged Gauteng tenders. A Marshalltown consultancy is named."));
	Send(TEXT("National Treasury"), FString::Printf(TEXT("Venter Consulting & Procurement is restricted from public tenders for %d days."), BlacklistDays));

	// A blacklisted company without a front is the moment Lerato calls back.
	if (!bHasFront) bFrontOffered = false;
}

void URANDTenderBoard::Send(const FString& Sender, const FString& Text)
{
	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this)) Phone->ReceiveMessage(Sender, FText::FromString(Text));
}

void URANDTenderBoard::LoadState(int32 InGrade, int32 InBEELevel, int32 InContractsWon, bool bInHasFront,
	int32 InBlacklistDays, float InWhistleblowerRisk)
{
	CIDBGrade = FMath::Clamp(InGrade, 1, 9);
	BEELevel = FMath::Clamp(InBEELevel, 0, 8);
	ContractsWon = FMath::Max(0, InContractsWon);
	bHasFront = bInHasFront;
	BlacklistDays = FMath::Max(0, InBlacklistDays);
	WhistleblowerRisk = FMath::Clamp(InWhistleblowerRisk, 0.f, 1.f);
	bFrontOffered = bHasFront;
	bFrontRetryTomorrow = !bHasFront;
	bTenderOpen = false;
	bFrontOfferOpen = false;

	// Whatever was on the phone belongs to the session we just left.
	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
	{
		Phone->WithdrawChoice(PassId);
		Phone->WithdrawChoice(FrontBuyId);
	}
}
