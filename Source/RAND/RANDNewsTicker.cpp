// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDNewsTicker.h"
#include "RANDCharacter.h"
#include "WantedComponent.h"
#include "TimeComponent.h"
#include "RANDPhoneWidget.h"
#include "Kismet/GameplayStatics.h"

URANDNewsTicker::URANDNewsTicker()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDNewsTicker::BeginPlay()
{
	Super::BeginPlay();
	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		Clock->OnHourPassed.AddDynamic(this, &URANDNewsTicker::HandleHourPassed);
	}
}

void URANDNewsTicker::HandleHourPassed(int32, int32 Hour)
{
	if (Hour % 3 != 0) return;
	Broadcast();
}

void URANDNewsTicker::Broadcast()
{
	URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this);
	if (!Phone) return;
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	const EHeatLevel Peak = (Player && Player->GetWantedComponent()) ? Player->GetWantedComponent()->GetPeakHeatLevel() : EHeatLevel::None;

	FString Line;
	switch (Peak)
	{
	case EHeatLevel::Burned:
		Line = TEXT("BREAKING: SAPS issues shoot-on-sight bulletin for a Marshalltown businessman.");
		break;
	case EHeatLevel::Manhunt:
		Line = TEXT("eNCA: Hawks raid linked to Tshwane fleet contracts. Roadblocks in the CBD.");
		break;
	case EHeatLevel::Wanted:
		Line = TEXT("702: Witnesses report gunfire near Park Station. SAPS asking for dashcam.");
		break;
	case EHeatLevel::Noticed:
		Line = TEXT("Business Day: Tender committee irregularities in Tshwane. Names withheld.");
		break;
	default:
		Line = TEXT("SABC: Another quiet morning in the CBD. Taxi associations deny a corridor war.");
		break;
	}
	Phone->ReceiveMessage(TEXT("News 24"), FText::FromString(Line));
}
