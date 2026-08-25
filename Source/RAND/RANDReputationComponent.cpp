// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDReputationComponent.h"

URANDReputationComponent::URANDReputationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Standing[IndexOf(ERANDContact::Sipho)] = 15.0f;
	Standing[IndexOf(ERANDContact::Thandi)] = 10.0f;
	Standing[IndexOf(ERANDContact::RankMarshal)] = 0.0f;
	Standing[IndexOf(ERANDContact::Lawyer)] = 5.0f;
}

void URANDReputationComponent::AddStanding(ERANDContact Contact, float Delta)
{
	SetStanding(Contact, GetStanding(Contact) + Delta);
}

void URANDReputationComponent::SetStanding(ERANDContact Contact, float Value)
{
	const int32 Idx = IndexOf(Contact);
	const float Clamped = FMath::Clamp(Value, 0.0f, 100.0f);
	if (FMath::IsNearlyEqual(Standing[Idx], Clamped))
	{
		return;
	}
	Standing[Idx] = Clamped;
	OnStandingChanged.Broadcast(Contact, Standing[Idx]);
}

float URANDReputationComponent::GetStanding(ERANDContact Contact) const
{
	return Standing[IndexOf(Contact)];
}

FString URANDReputationComponent::GetContactName(ERANDContact Contact) const
{
	switch (Contact)
	{
	case ERANDContact::Sipho:        return TEXT("Sipho Dlamini");
	case ERANDContact::Thandi:       return TEXT("Thandi Mokoena");
	case ERANDContact::RankMarshal:  return TEXT("Park Station Marshal");
	case ERANDContact::Lawyer:       return TEXT("Advocate Naidoo");
	default:                         return TEXT("Contact");
	}
}

void URANDReputationComponent::LoadStandings(const TArray<float>& InStandings)
{
	for (int32 i = 0; i < ContactCount && i < InStandings.Num(); ++i)
	{
		Standing[i] = FMath::Clamp(InStandings[i], 0.0f, 100.0f);
	}
}

void URANDReputationComponent::CopyStandings(TArray<float>& OutStandings) const
{
	OutStandings.Reset(ContactCount);
	for (int32 i = 0; i < ContactCount; ++i)
	{
		OutStandings.Add(Standing[i]);
	}
}
