// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDFactionComponent.h"
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

namespace
{
	/** One standing deal per faction. Cash > 0 is paid to André, < 0 is his buy-in. */
	struct FFactionOffer
	{
		const TCHAR* Sender;
		const TCHAR* Body;
		const TCHAR* AcceptLabel;
		float Cash;
		float CleanReturn;		// Paid back on top when a buy-in clears (laundering).
		EAgency HeatAgency;
		float Heat;
		float AcceptStanding;
		float RefuseStanding;
		const TCHAR* AcceptReply;
		const TCHAR* RefuseReply;
	};

	// Indexed by ERANDFaction.
	const FFactionOffer Offers[] =
	{
		{ TEXT("Chief (Hillbrow)"),
		  TEXT("We move money through your company. You take a cut and ask nothing. R40k this week."),
		  TEXT("Take the cut — R40,000"), 40000.f, 0.f, EAgency::Hawks, 8.f, 45.f, -30.f,
		  TEXT("Good. Hillbrow remembers friends."),
		  TEXT("You chose. Don't park on Pretoria Street.") },
		{ TEXT("Uncle Faizel"),
		  TEXT("Product from the Flats needs a road north. Your logistics trucks, one run. R30k."),
		  TEXT("Lend the trucks — R30,000"), 30000.f, 0.f, EAgency::SAPS, 6.f, 20.f, -10.f,
		  TEXT("Driver says the N1 was quiet. Lekker."),
		  TEXT("Shame. The road is long without friends.") },
		{ TEXT("Tendai"),
		  TEXT("Crates from Ressano Garcia. We need a Centurion warehouse for three nights. R35k."),
		  TEXT("Store the crates — R35,000"), 35000.f, 0.f, EAgency::Hawks, 6.f, 20.f, -10.f,
		  TEXT("Crates gone. Nobody counted them."),
		  TEXT("Okay. We find another roof.") },
		{ TEXT("Dmitri"),
		  TEXT("Put R60k into our Sandton development. It comes back as R90k, clean, with invoices."),
		  TEXT("Invest R60,000 — take R90,000 clean"), -60000.f, 90000.f, EAgency::Hawks, 4.f, 20.f, -5.f,
		  TEXT("Paperwork is in your inbox. Consultancy fees. Very boring."),
		  TEXT("A pity. Money that stays dirty stays loud.") },
		{ TEXT("Rank Chairman"),
		  TEXT("Your cars use our routes. R20k for the association and the marshals tell you what SAPS is doing."),
		  TEXT("Pay the association — R20,000"), -20000.f, 0.f, EAgency::SAPS, -10.f, 20.f, -25.f,
		  TEXT("The marshals are watching the roadblocks for you now."),
		  TEXT("Then the rank doesn't know you. Neither do the roads.") },
		{ TEXT("Unknown number"),
		  TEXT("Gold from under Marshalltown. We need your warehouse furnace one night. R28k."),
		  TEXT("Rent the furnace — R28,000"), 28000.f, 0.f, EAgency::SAPS, 8.f, 20.f, -10.f,
		  TEXT("Bars are cooling. Your share is in the car."),
		  TEXT("We have other furnaces. Watch the shafts.") },
	};
	static_assert(UE_ARRAY_COUNT(Offers) == 6, "One offer per ERANDFaction");

	const FName AcceptId(TEXT("Faction_Accept"));
	const FName RefuseId(TEXT("Faction_Refuse"));

	/** Offers left unanswered this long lapse so the next faction can call. */
	constexpr int32 OfferExpiryHours = 12;
}

URANDFactionComponent::URANDFactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

URANDFactionComponent* URANDFactionComponent::Get(const UObject* WorldContext)
{
	UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (!World) return nullptr;
	if (ARANDGameMode* GM = World->GetAuthGameMode<ARANDGameMode>())
	{
		return GM->GetFactions();
	}
	return nullptr;
}

void URANDFactionComponent::BeginPlay()
{
	Super::BeginPlay();
	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		Clock->OnHourPassed.AddDynamic(this, &URANDFactionComponent::HandleHourPassed);
	}
	BindPhone();
}

void URANDFactionComponent::BindPhone()
{
	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
	{
		Phone->OnMessageOptionSelected.AddDynamic(this, &URANDFactionComponent::HandlePhoneOption);
	}
	else if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(BindTimer, this, &URANDFactionComponent::BindPhone, 0.5f, false);
	}
}

float URANDFactionComponent::GetStanding(ERANDFaction Faction) const
{
	const int32 Index = IndexOf(Faction);
	return (Index >= 0 && Index < FactionCount) ? Standing[Index] : 0.f;
}

void URANDFactionComponent::AddStanding(ERANDFaction Faction, float Delta)
{
	SetStandingInternal(Faction, GetStanding(Faction) + Delta);
}

void URANDFactionComponent::SetStandingInternal(ERANDFaction Faction, float Value)
{
	const int32 Index = IndexOf(Faction);
	if (Index < 0 || Index >= FactionCount) return;

	const ERANDFactionRelation OldRelation = Bucket(Standing[Index]);
	Standing[Index] = FMath::Clamp(Value, -100.f, 100.f);
	const ERANDFactionRelation NewRelation = Bucket(Standing[Index]);
	if (OldRelation != NewRelation)
	{
		OnRelationChanged.Broadcast(Faction, OldRelation, NewRelation);
	}
}

ERANDFactionRelation URANDFactionComponent::GetRelation(ERANDFaction Faction) const
{
	return Bucket(GetStanding(Faction));
}

ERANDFactionRelation URANDFactionComponent::Bucket(float Value)
{
	if (Value <= -50.f) return ERANDFactionRelation::War;
	if (Value <= -15.f) return ERANDFactionRelation::Hostile;
	if (Value < 40.f)   return ERANDFactionRelation::Neutral;
	if (Value < 75.f)   return ERANDFactionRelation::Partner;
	return ERANDFactionRelation::Allied;
}

FString URANDFactionComponent::GetFactionName(ERANDFaction Faction)
{
	switch (Faction)
	{
	case ERANDFaction::NigerianNetworks: return TEXT("Nigerian Networks");
	case ERANDFaction::CapeFlatsReach:   return TEXT("Cape Flats Reach");
	case ERANDFaction::ZimMozNetworks:   return TEXT("Zim/Moz Networks");
	case ERANDFaction::EasternEuropean:  return TEXT("Eastern European");
	case ERANDFaction::TaxiIndustry:     return TEXT("Taxi Industry");
	case ERANDFaction::ZamaZamas:        return TEXT("Zama Zamas");
	default:                             return TEXT("Unknown");
	}
}

int32 URANDFactionComponent::GetWarCount() const
{
	int32 Count = 0;
	for (int32 i = 0; i < FactionCount; ++i)
	{
		if (Bucket(Standing[i]) == ERANDFactionRelation::War) ++Count;
	}
	return Count;
}

void URANDFactionComponent::HandleHourPassed(int32, int32)
{
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (Player)
	{
		const int32 Wars = GetWarCount();
		if (Wars > 0 && Player->GetWantedComponent())
		{
			Player->GetWantedComponent()->AddHeat(EAgency::Rivals, WarHeatPerHour * Wars);
		}

		int32 Allies = 0;
		for (int32 i = 0; i < FactionCount; ++i)
		{
			if (Bucket(Standing[i]) == ERANDFactionRelation::Allied) ++Allies;
		}
		if (Allies > 0 && Player->GetEconomyComponent())
		{
			Player->GetEconomyComponent()->AddFunds(AlliedTributePerHour * Allies, TEXT("Faction tribute share"));
		}
	}

	if (bOfferOpen)
	{
		if (++HoursOfferOpen >= OfferExpiryHours)
		{
			// Silence is an answer too -- a quieter one than refusing.
			bOfferOpen = false;
			AddStanding(PendingFaction, Offers[IndexOf(PendingFaction)].RefuseStanding * 0.5f);
			if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
			{
				Phone->WithdrawChoice(AcceptId);
				Phone->ReceiveMessage(Offers[IndexOf(PendingFaction)].Sender, FText::FromString(TEXT("Offer's off the table.")));
			}
		}
		return;
	}

	if (++HoursSinceOffer >= OfferIntervalHours) MakeOffer();
}

void URANDFactionComponent::MakeOffer()
{
	URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this);
	if (!Phone || Phone->HasPendingChoice()) return; // Try again next hour.

	// Factions at War don't do business; everyone else is fair game.
	TArray<ERANDFaction> Candidates;
	for (int32 i = 0; i < FactionCount; ++i)
	{
		if (Bucket(Standing[i]) != ERANDFactionRelation::War) Candidates.Add(static_cast<ERANDFaction>(i));
	}
	if (Candidates.Num() == 0) return;

	PendingFaction = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
	const FFactionOffer& Offer = Offers[IndexOf(PendingFaction)];
	HoursSinceOffer = 0;
	HoursOfferOpen = 0;
	bOfferOpen = true;

	TArray<FRANDMessageOption> Options;
	FRANDMessageOption A; A.Label = FText::FromString(Offer.AcceptLabel); A.ActionId = AcceptId; Options.Add(A);
	FRANDMessageOption B; B.Label = FText::FromString(TEXT("Refuse")); B.ActionId = RefuseId; Options.Add(B);
	Phone->ReceiveMessageWithOptions(Offer.Sender, FText::FromString(Offer.Body), Options);
}

void URANDFactionComponent::HandlePhoneOption(FName ActionId)
{
	if (!bOfferOpen) return;
	if (ActionId == AcceptId) Resolve(true);
	else if (ActionId == RefuseId) Resolve(false);
}

void URANDFactionComponent::Resolve(bool bAccepted)
{
	bOfferOpen = false;
	const FFactionOffer& Offer = Offers[IndexOf(PendingFaction)];
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this);
	if (!Player) return;

	if (!bAccepted)
	{
		AddStanding(PendingFaction, Offer.RefuseStanding);
		if (Phone) Phone->ReceiveMessage(Offer.Sender, FText::FromString(Offer.RefuseReply));
		return;
	}

	URANDEconomyComponent* Econ = Player->GetEconomyComponent();
	if (!Econ) return;
	URANDSARSComponent* SARS = URANDSARSComponent::Get(this);
	const FString Source = FString::Printf(TEXT("Deal: %s"), *GetFactionName(PendingFaction));

	if (Offer.Cash < 0.f)
	{
		const float BuyIn = SARS ? SARS->Adjust(-Offer.Cash) : -Offer.Cash;
		if (!Econ->DeductFunds(BuyIn, Source))
		{
			if (Phone) Phone->ReceiveMessage(Offer.Sender, FText::FromString(TEXT("Come back when the money is real.")));
			return;
		}
	}
	else if (Offer.Cash > 0.f)
	{
		Econ->AddFunds(SARS ? SARS->Adjust(Offer.Cash) : Offer.Cash, Source);
	}
	if (Offer.CleanReturn > 0.f)
	{
		Econ->AddFunds(SARS ? SARS->Adjust(Offer.CleanReturn) : Offer.CleanReturn, Source + TEXT(" (clean return)"));
	}

	if (UWantedComponent* Wanted = Player->GetWantedComponent())
	{
		if (Offer.Heat > 0.f)
		{
			Wanted->AddHeat(Offer.HeatAgency, Offer.Heat);
		}
		else if (Offer.Heat < 0.f)
		{
			// Negative heat is a favour (e.g. marshals tipping off roadblocks).
			Wanted->SetHeat(Offer.HeatAgency, FMath::Max(0.f, Wanted->GetRawHeat(Offer.HeatAgency) + Offer.Heat));
		}
	}

	AddStanding(PendingFaction, Offer.AcceptStanding);
	if (Phone) Phone->ReceiveMessage(Offer.Sender, FText::FromString(Offer.AcceptReply));
}

void URANDFactionComponent::LoadStandings(const TArray<float>& InStandings)
{
	for (int32 i = 0; i < FactionCount; ++i)
	{
		SetStandingInternal(static_cast<ERANDFaction>(i), InStandings.IsValidIndex(i) ? InStandings[i] : 0.f);
	}
}

void URANDFactionComponent::CopyStandings(TArray<float>& OutStandings) const
{
	OutStandings.SetNum(FactionCount);
	for (int32 i = 0; i < FactionCount; ++i) OutStandings[i] = Standing[i];
}
