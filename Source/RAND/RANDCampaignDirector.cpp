// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDCampaignDirector.h"
#include "RANDCharacter.h"
#include "RANDCharacter_NPC.h"
#include "RANDMissionManager.h"
#include "RANDMission_Consultation.h"
#include "EconomyComponent.h"
#include "BusinessManager.h"
#include "WantedComponent.h"
#include "RANDCareerComponent.h"
#include "RANDReputationComponent.h"
#include "RANDCombatComponent.h"
#include "RANDSARSComponent.h"
#include "RANDPhoneWidget.h"
#include "DialogueComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

const FName URANDCampaignDirector::Mission2ID = TEXT("M2_SpecWriting");
const FName URANDCampaignDirector::Mission3ID = TEXT("M3_ParkStation");
const FName URANDCampaignDirector::Mission4ID = TEXT("M4_Braai");
const FName URANDCampaignDirector::Mission5ID = TEXT("M5_HawksAtTheDoor");

URANDCampaignDirector::URANDCampaignDirector()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDCampaignDirector::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(KickTimer, this, &URANDCampaignDirector::Kickoff, 1.0f, false);
	}
}

void URANDCampaignDirector::Kickoff()
{
	SeedStartingLife();
	EnsureConsultationExists();
	BindPhone();
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this))
	{
		Missions->OnMissionComplete.AddDynamic(this, &URANDCampaignDirector::HandleMissionComplete);
	}
	Send(TEXT("Sipho Dlamini"), NSLOCTEXT("RANDCampaign", "Kick", "Bra Andre — the Tshwane file is on your desk. Come through to Marshalltown when you're ready."));
}

void URANDCampaignDirector::SeedStartingLife()
{
	if (bSeeded) return;
	bSeeded = true;
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player) return;
	if (URANDEconomyComponent* Econ = Player->GetEconomyComponent())
	{
		if (Econ->GetBalance() <= 0.0f)
		{
			Econ->AddFunds(125000.0f, TEXT("Operating capital — Venter Consulting & Procurement"));
		}
	}
	if (URANDBusinessManager* Biz = Player->GetBusinessManager())
	{
		if (Biz->GetBusinesses().Num() == 0)
		{
			FRANDBusiness VCP;
			VCP.BusinessName = TEXT("Venter Consulting & Procurement");
			VCP.BusinessType = ERANDBusinessType::Legitimate;
			VCP.PassiveIncomePerHour = 850.0f;
			VCP.bIsActive = true;
			Biz->AddBusiness(VCP);
		}
	}
}

void URANDCampaignDirector::EnsureConsultationExists()
{
	if (UGameplayStatics::GetActorOfClass(this, ARANDMission_Consultation::StaticClass())) return;
	UWorld* World = GetWorld();
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!World || !Player) return;
	World->SpawnActor<ARANDMission_Consultation>(Player->GetActorLocation() + FVector(1200,0,0), FRotator::ZeroRotator);
}

void URANDCampaignDirector::BindPhone()
{
	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
	{
		Phone->OnMessageOptionSelected.AddDynamic(this, &URANDCampaignDirector::HandlePhoneOption);
	}
	else if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(MessageTimer, this, &URANDCampaignDirector::BindPhone, 0.5f, false);
	}
}

void URANDCampaignDirector::HandleMissionComplete(FName MissionID)
{
	if (MissionID == ARANDMission_Consultation::MissionID) { ActIndex = 2; StartMission2(); }
	else if (MissionID == Mission2ID) { ActIndex = 3; StartMission3(); }
	else if (MissionID == Mission3ID) { ActIndex = 4; StartMission4(); }
	else if (MissionID == Mission4ID) { ActIndex = 5; StartMission5(); }
	else if (MissionID == Mission5ID) { FinishCampaign(); }
}

void URANDCampaignDirector::StartMission2()
{
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	const bool bGrey = Player && Player->bAcceptedBribe;
	FRANDMission M;
	M.MissionID = Mission2ID;
	M.MissionName = bGrey ? FText::FromString(TEXT("Spec Writing")) : FText::FromString(TEXT("Clean Work"));
	FRANDObjective O;
	O.ObjectiveText = bGrey ? FText::FromString(TEXT("Talk to Thandi — lock the spec")) : FText::FromString(TEXT("Talk to Sipho — another legitimate facilitation"));
	M.Objectives.Add(O);
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this))
	{
		Missions->RegisterMission(M);
		Missions->StartMission(Mission2ID);
	}
	if (bGrey)
	{
		if (Player && Player->GetCareerComponent()) Player->GetCareerComponent()->SetStage(ERANDCareerStage::Player);
		Send(TEXT("Thandi Mokoena"), FText::FromString(TEXT("The next one is a R12m clinic upgrade. I can write the spec around your boy's plant. Come see me.")));
		SpawnTalkObjective(TEXT("Thandi Mokoena"), FText::FromString(TEXT("I'll file the spec tonight. Don't text this number from your real SIM.")), Mission2ID);
	}
	else
	{
		Send(TEXT("Sipho Dlamini"), FText::FromString(TEXT("There's another road package in Tshwane. Same 8%. No funny business.")));
		SpawnTalkObjective(TEXT("Sipho Dlamini"), FText::FromString(TEXT("Sharp. We stay clean, we stay small.")), Mission2ID);
	}
}

void URANDCampaignDirector::StartMission3()
{
	FRANDMission M; M.MissionID = Mission3ID; M.MissionName = FText::FromString(TEXT("Park Station"));
	FRANDObjective O; O.ObjectiveText = FText::FromString(TEXT("Talk to the rank marshal")); M.Objectives.Add(O);
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) { Missions->RegisterMission(M); Missions->StartMission(Mission3ID); }
	Send(TEXT("Sipho Dlamini"), FText::FromString(TEXT("If you want things to move between Joburg and Pretoria you talk to the rank. Ask for Bra Mike.")));
	SpawnTalkObjective(TEXT("Bra Mike"), FText::FromString(TEXT("Taxis move what taxis move. Phone's in your pocket when you're ready.")), Mission3ID);
	SendChoice(TEXT("Bra Mike"), FText::FromString(TEXT("R40k and you get a route. Or stay a consultant and walk.")),
		FText::FromString(TEXT("Pay R40,000 — open the corridor")), TEXT("M3_PayRank"),
		FText::FromString(TEXT("Walk away")), TEXT("M3_Walk"));
}

void URANDCampaignDirector::StartMission4()
{
	FRANDMission M; M.MissionID = Mission4ID; M.MissionName = FText::FromString(TEXT("The Braai"));
	FRANDObjective O; O.ObjectiveText = FText::FromString(TEXT("Go to Sipho's braai")); M.Objectives.Add(O);
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) { Missions->RegisterMission(M); Missions->StartMission(Mission4ID); }
	Send(TEXT("Sipho Dlamini"), FText::FromString(TEXT("Sunday. My place. We don't talk tenders at the fire — we talk who we trust.")));
	SpawnTalkObjective(TEXT("Sipho Dlamini"), FText::FromString(TEXT("Lekker. Now they know your face.")), Mission4ID);
}

void URANDCampaignDirector::StartMission5()
{
	FRANDMission M; M.MissionID = Mission5ID; M.MissionName = FText::FromString(TEXT("Hawks at the Door"));
	FRANDObjective O; O.ObjectiveText = FText::FromString(TEXT("Answer Advocate Naidoo")); M.Objectives.Add(O);
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) { Missions->RegisterMission(M); Missions->StartMission(Mission5ID); }
	SendChoice(TEXT("Advocate Naidoo"), FText::FromString(TEXT("DPCI opened a file. I can stall the NPA for R120k. Or you disappear for a week.")),
		FText::FromString(TEXT("Pay R120,000 — stall the NPA")), TEXT("M5_Lawyer"),
		FText::FromString(TEXT("Go dark (F6)")), TEXT("M5_Dark"));
}

void URANDCampaignDirector::FinishCampaign()
{
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player || !Player->GetCareerComponent()) return;
	URANDCareerComponent* Career = Player->GetCareerComponent();
	const bool bGrey = Player->bAcceptedBribe;
	const float Cash = Player->GetEconomyComponent() ? Player->GetEconomyComponent()->GetBalance() : 0.f;
	const EHeatLevel Peak = Player->GetWantedComponent() ? Player->GetWantedComponent()->GetPeakHeatLevel() : EHeatLevel::None;
	ERANDEnding Ending = ERANDEnding::Legitimate;
	if (Peak >= EHeatLevel::Burned)
	{
		Ending = ERANDEnding::Destroyed;
		Send(TEXT("Advocate Naidoo"), FText::FromString(TEXT("They're at Marshalltown. Don't come in. The city does not belong to you.")));
	}
	else if (bGrey && Cash >= 200000.f)
	{
		Ending = ERANDEnding::StateCapture;
		Career->SetStage(ERANDCareerStage::Operator);
		Send(TEXT("Thandi Mokoena"), FText::FromString(TEXT("The committee is ours. Next year we don't bribe them. We appoint them.")));
	}
	else
	{
		Ending = ERANDEnding::Legitimate;
		Career->SetStage(ERANDCareerStage::Player);
		Send(TEXT("Sipho Dlamini"), FText::FromString(TEXT("You didn't take the short road. The money is smaller. You still sleep.")));
	}
	Career->ResolveEnding(Ending);
}

void URANDCampaignDirector::SpawnTalkObjective(const FString& NPCName, const FText& Line, FName)
{
	UWorld* World = GetWorld();
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!World || !Player) return;
	if (IsValid(ActiveNPC)) { ActiveNPC->Destroy(); ActiveNPC = nullptr; }
	const FVector Loc = Player->GetActorLocation() + Player->GetActorForwardVector() * 450.f;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ActiveNPC = World->SpawnActor<ARANDCharacter_NPC>(ARANDCharacter_NPC::StaticClass(), Loc, FRotator::ZeroRotator, Params);
	if (!ActiveNPC) return;
	ActiveNPC->NPCName = NPCName;
	ActiveNPC->NPCType = ENPCType::Contact;
	if (URANDDialogueComponent* Dialogue = ActiveNPC->GetDialogueComponent())
	{
		FRANDDialogueLine L; L.Speaker = NPCName; L.Line = Line;
		Dialogue->DialogueLines.Reset(); Dialogue->DialogueLines.Add(L);
		Dialogue->OnDialogueStarted.AddDynamic(this, &URANDCampaignDirector::HandleDialogueStarted);
		Dialogue->OnDialogueEnded.AddDynamic(this, &URANDCampaignDirector::CompleteActiveTalk);
	}
}

void URANDCampaignDirector::HandleDialogueStarted(const FRANDDialogueLine&)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(MessageTimer, FTimerDelegate::CreateLambda([this]()
		{
			if (IsValid(ActiveNPC) && ActiveNPC->GetDialogueComponent())
			{
				ActiveNPC->GetDialogueComponent()->EndDialogue();
			}
		}), 4.0f, false);
	}
}

void URANDCampaignDirector::CompleteActiveTalk()
{
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this))
	{
		const FName Active = Missions->GetActiveMissionID();
		if (Active != NAME_None) Missions->CompleteObjective(Active, 0);
	}
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player) return;
	if (URANDReputationComponent* Rep = Player->GetReputationComponent())
	{
		Rep->AddStanding(ERANDContact::Sipho, ActIndex == 4 ? 28.f : 8.f);
	}
	if (ActIndex == 2 && Player->GetEconomyComponent())
	{
		float Payout = Player->bAcceptedBribe ? 180000.f : 33600.f;
		if (URANDSARSComponent* SARS = URANDSARSComponent::Get(this)) Payout = SARS->Adjust(Payout);
		Player->GetEconomyComponent()->AddFunds(Payout, Player->bAcceptedBribe ? TEXT("Tender: Tshwane clinic upgrade") : TEXT("Facilitation fee: Tshwane roads"));
	}
}

void URANDCampaignDirector::HandlePhoneOption(FName ActionId)
{
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player) return;
	if (ActionId == TEXT("M3_PayRank"))
	{
		const bool bPaid = Player->GetEconomyComponent() && Player->GetEconomyComponent()->DeductFunds(40000.f, TEXT("Park Station — rank seat"));
		if (bPaid)
		{
			if (URANDBusinessManager* Biz = Player->GetBusinessManager())
			{
				FRANDBusiness Route; Route.BusinessName = TEXT("Corridor Logistics (Park Station)");
				Route.BusinessType = ERANDBusinessType::Grey; Route.PassiveIncomePerHour = 4200.f; Route.HeatGenerationRate = 1.5f; Route.bIsActive = true;
				Biz->AddBusiness(Route);
			}
			if (URANDCombatComponent* Combat = Player->GetCombatComponent()) Combat->SetArmed(true);
			if (URANDReputationComponent* Rep = Player->GetReputationComponent()) Rep->AddStanding(ERANDContact::RankMarshal, 40.f);
			if (Player->GetWantedComponent()) Player->GetWantedComponent()->AddHeat(EAgency::Rivals, 12.f);
			Send(TEXT("Bra Mike"), FText::FromString(TEXT("You're on the board. There's a piece in the cubby if the corridor gets ugly.")));
		}
		else Send(TEXT("Bra Mike"), FText::FromString(TEXT("Come back when you have actual money.")));
	}
	else if (ActionId == TEXT("M3_Walk"))
	{
		Send(TEXT("Bra Mike"), FText::FromString(TEXT("Wise. Or small. We'll see which.")));
	}
	else if (ActionId == TEXT("M5_Lawyer"))
	{
		if (Player->GetEconomyComponent() && Player->GetEconomyComponent()->DeductFunds(120000.f, TEXT("Advocate Naidoo — stall NPA")))
		{
			if (UWantedComponent* Wanted = Player->GetWantedComponent())
			{
				Wanted->SetHeat(EAgency::Hawks, FMath::Max(0.f, Wanted->GetRawHeat(EAgency::Hawks) - 35.f));
			}
			if (URANDReputationComponent* Rep = Player->GetReputationComponent()) Rep->AddStanding(ERANDContact::Lawyer, 25.f);
			Send(TEXT("Advocate Naidoo"), FText::FromString(TEXT("The docket will sit on a desk in Pretoria until after the recess.")));
		}
		if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) Missions->CompleteObjective(Mission5ID, 0);
	}
	else if (ActionId == TEXT("M5_Dark"))
	{
		if (URANDCareerComponent* Career = Player->GetCareerComponent()) Career->GoDark();
		if (URANDMissionManager* Missions = URANDMissionManager::Get(this)) Missions->CompleteObjective(Mission5ID, 0);
	}
}

void URANDCampaignDirector::Send(const FString& Sender, const FText& Text)
{
	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this)) Phone->ReceiveMessage(Sender, Text);
}

void URANDCampaignDirector::SendChoice(const FString& Sender, const FText& Text, const FText& ALabel, FName AId, const FText& BLabel, FName BId)
{
	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
	{
		TArray<FRANDMessageOption> Options;
		FRANDMessageOption A; A.Label = ALabel; A.ActionId = AId; Options.Add(A);
		FRANDMessageOption B; B.Label = BLabel; B.ActionId = BId; Options.Add(B);
		Phone->ReceiveMessageWithOptions(Sender, Text, Options);
	}
}
