// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDAct3Director.h"
#include "RANDCharacter.h"
#include "EconomyComponent.h"
#include "BusinessManager.h"
#include "WantedComponent.h"
#include "RANDCareerComponent.h"
#include "RANDInventoryComponent.h"
#include "RANDMissionManager.h"
#include "RANDPhoneWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

const FName URANDAct3Director::Mission9ID  = TEXT("M9_Ponte");
const FName URANDAct3Director::Mission10ID = TEXT("M10_Interpol");
const FName URANDAct3Director::Mission11ID = TEXT("M11_Cabinet");

URANDAct3Director::URANDAct3Director()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDAct3Director::BeginPlay()
{
	Super::BeginPlay();
	Bind();
}

void URANDAct3Director::Bind()
{
	bool bReady = false;
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this))
	{
		Missions->OnMissionComplete.AddDynamic(this, &URANDAct3Director::HandleMissionComplete);
		bReady = true;
	}
	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
	{
		Phone->OnMessageOptionSelected.AddDynamic(this, &URANDAct3Director::HandlePhoneOption);
	}
	else if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(BindTimer, this, &URANDAct3Director::Bind, 0.5f, false);
	}
	(void)bReady;
}

void URANDAct3Director::HandleMissionComplete(FName MissionID)
{
	if (MissionID == TEXT("M8_NPA")) StartMission9();
	else if (MissionID == Mission9ID) StartMission10();
	else if (MissionID == Mission10ID) StartMission11();
	else if (MissionID == Mission11ID) Finish();
}

void URANDAct3Director::StartMission9()
{
	FRANDMission M; M.MissionID = Mission9ID; M.MissionName = FText::FromString(TEXT("Ponte"));
	FRANDObjective O; O.ObjectiveText = FText::FromString(TEXT("Take or refuse the Berea floor")); M.Objectives.Add(O);
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) { Missions->RegisterMission(M); Missions->StartMission(Mission9ID); }
	SendChoice(TEXT("Unknown number"),
		FText::FromString(TEXT("Ponte. Whole floor. R350k. You see the CBD from the window and nobody sees you." )),
		FText::FromString(TEXT("Buy the floor — R350,000")), TEXT("M9_Buy"),
		FText::FromString(TEXT("Leave Berea alone")), TEXT("M9_Skip"));
}

void URANDAct3Director::StartMission10()
{
	FRANDMission M; M.MissionID = Mission10ID; M.MissionName = FText::FromString(TEXT("Red notice"));
	FRANDObjective O; O.ObjectiveText = FText::FromString(TEXT("Answer Interpol through Naidoo")); M.Objectives.Add(O);
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) { Missions->RegisterMission(M); Missions->StartMission(Mission10ID); }
	SendChoice(TEXT("Advocate Naidoo"),
		FText::FromString(TEXT("Lyon asked Pretoria for a file. I can lose the request for R300k. Or we let it land." )),
		FText::FromString(TEXT("Pay R300,000 — lose the request")), TEXT("M10_Pay"),
		FText::FromString(TEXT("Let it land")), TEXT("M10_Land"));
}

void URANDAct3Director::StartMission11()
{
	FRANDMission M; M.MissionID = Mission11ID; M.MissionName = FText::FromString(TEXT("Cabinet"));
	FRANDObjective O; O.ObjectiveText = FText::FromString(TEXT("Take the appointment or walk")); M.Objectives.Add(O);
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) { Missions->RegisterMission(M); Missions->StartMission(Mission11ID); }
	SendChoice(TEXT("Thandi Mokoena"),
		FText::FromString(TEXT("There is a seat on the procurement council. You don't apply. You are appointed. Or you stay a contractor." )),
		FText::FromString(TEXT("Take the seat")), TEXT("M11_Seat"),
		FText::FromString(TEXT("Stay a contractor")), TEXT("M11_Walk"));
}

void URANDAct3Director::Finish()
{
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player || !Player->GetCareerComponent()) return;
	const EHeatLevel Peak = Player->GetWantedComponent() ? Player->GetWantedComponent()->GetPeakHeatLevel() : EHeatLevel::None;
	if (Peak >= EHeatLevel::Burned)
	{
		Player->GetCareerComponent()->ResolveEnding(ERANDEnding::Destroyed);
		Send(TEXT("Advocate Naidoo"), FText::FromString(TEXT("Red notice. Don't use your name.")));
		return;
	}
	if (Player->GetCareerComponent()->GetStage() == ERANDCareerStage::TheCityIsYours)
	{
		Player->GetCareerComponent()->ResolveEnding(ERANDEnding::StateCapture);
		Send(TEXT("Thandi Mokoena"), FText::FromString(TEXT("The city is a spreadsheet. Your name is the header.")));
	}
	else
	{
		Player->GetCareerComponent()->ResolveEnding(ERANDEnding::Legitimate);
		Send(TEXT("Sipho Dlamini"), FText::FromString(TEXT("You could have taken the building. You took the door instead.")));
	}
}

void URANDAct3Director::HandlePhoneOption(FName ActionId)
{
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player) return;

	if (ActionId == TEXT("M9_Buy"))
	{
		if (Player->GetEconomyComponent() && Player->GetEconomyComponent()->DeductFunds(350000.f, TEXT("Ponte floor")))
		{
			if (URANDBusinessManager* Biz = Player->GetBusinessManager())
			{
				FRANDBusiness Floor; Floor.BusinessName = TEXT("Ponte — Berea floor");
				Floor.BusinessType = ERANDBusinessType::Grey; Floor.PassiveIncomePerHour = 15000.f; Floor.HeatGenerationRate = 2.0f; Floor.bIsActive = true;
				Biz->AddBusiness(Floor);
			}
			Send(TEXT("Unknown number"), FText::FromString(TEXT("The lift only stops if we let it.")));
		}
		if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) Missions->CompleteObjective(Mission9ID, 0);
	}
	else if (ActionId == TEXT("M9_Skip"))
	{
		Send(TEXT("Unknown number"), FText::FromString(TEXT("Then don't look up when you drive past.")));
		if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) Missions->CompleteObjective(Mission9ID, 0);
	}
	else if (ActionId == TEXT("M10_Pay"))
	{
		if (Player->GetEconomyComponent() && Player->GetEconomyComponent()->DeductFunds(300000.f, TEXT("Interpol request")))
		{
			if (UWantedComponent* Wanted = Player->GetWantedComponent())
			{
				Wanted->SetHeat(EAgency::Hawks, FMath::Max(0.f, Wanted->GetRawHeat(EAgency::Hawks) - 25.f));
			}
			Send(TEXT("Advocate Naidoo"), FText::FromString(TEXT("Lyon will get a blank page. For a while.")));
		}
		if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) Missions->CompleteObjective(Mission10ID, 0);
	}
	else if (ActionId == TEXT("M10_Land"))
	{
		if (Player->GetWantedComponent()) Player->GetWantedComponent()->AddHeat(EAgency::Hawks, 20.f);
		Send(TEXT("Advocate Naidoo"), FText::FromString(TEXT("Then we wait for the envelope.")));
		if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) Missions->CompleteObjective(Mission10ID, 0);
	}
	else if (ActionId == TEXT("M11_Seat"))
	{
		if (URANDCareerComponent* Career = Player->GetCareerComponent()) Career->SetStage(ERANDCareerStage::TheCityIsYours);
		Send(TEXT("Thandi Mokoena"), FText::FromString(TEXT("You don't bid. You appoint. Welcome to the table.")));
		if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) Missions->CompleteObjective(Mission11ID, 0);
	}
	else if (ActionId == TEXT("M11_Walk"))
	{
		Send(TEXT("Thandi Mokoena"), FText::FromString(TEXT("Then stay hungry. The table does not wait.")));
		if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) Missions->CompleteObjective(Mission11ID, 0);
	}
}

void URANDAct3Director::Send(const FString& Sender, const FText& Text)
{
	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this)) Phone->ReceiveMessage(Sender, Text);
}

void URANDAct3Director::SendChoice(const FString& Sender, const FText& Text, const FText& A, FName AId, const FText& B, FName BId)
{
	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
	{
		TArray<FRANDMessageOption> Options;
		FRANDMessageOption OptA; OptA.Label = A; OptA.ActionId = AId; Options.Add(OptA);
		FRANDMessageOption OptB; OptB.Label = B; OptB.ActionId = BId; Options.Add(OptB);
		Phone->ReceiveMessageWithOptions(Sender, Text, Options);
	}
}
