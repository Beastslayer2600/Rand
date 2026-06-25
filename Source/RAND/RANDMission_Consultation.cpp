// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDMission_Consultation.h"
#include "RANDMissionManager.h"
#include "RANDCharacter.h"
#include "RANDCharacter_NPC.h"
#include "RANDTenderDesk.h"
#include "DialogueComponent.h"
#include "EconomyComponent.h"
#include "WantedComponent.h"
#include "RANDPhoneWidget.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

const FName ARANDMission_Consultation::MissionID = FName(TEXT("Mission_Consultation"));

ARANDMission_Consultation::ARANDMission_Consultation()
{
	PrimaryActorTick.bCanEverTick = false;

	OfficeTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("OfficeTrigger"));
	SetRootComponent(OfficeTrigger);
	OfficeTrigger->SetSphereRadius(OfficeTriggerRadius);
	OfficeTrigger->SetCollisionProfileName(TEXT("Trigger"));
	OfficeTrigger->SetGenerateOverlapEvents(true);
}

void ARANDMission_Consultation::BeginPlay()
{
	Super::BeginPlay();

	OfficeTrigger->SetSphereRadius(OfficeTriggerRadius);
	OfficeTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARANDMission_Consultation::HandleOfficeOverlap);

	// Register and start the mission.
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this))
	{
		FRANDMission Mission;
		Mission.MissionID = MissionID;
		Mission.MissionName = NSLOCTEXT("RANDMission", "Consultation", "The Consultation");

		auto AddObjective = [&Mission](const FText& Text)
		{
			FRANDObjective Obj;
			Obj.ObjectiveText = Text;
			Mission.Objectives.Add(Obj);
		};
		AddObjective(NSLOCTEXT("RANDMission", "Obj_Drive",   "Drive to the Marshalltown office"));
		AddObjective(NSLOCTEXT("RANDMission", "Obj_Thandi",  "Speak to Thandi"));
		AddObjective(NSLOCTEXT("RANDMission", "Obj_Submit",  "Submit the tender"));
		AddObjective(NSLOCTEXT("RANDMission", "Obj_Respond", "Respond to Thandi's offer"));

		Missions->RegisterMission(Mission);
		Missions->StartMission(MissionID);
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Deliver the opening briefing by phone once the HUD/phone exists.
	World->GetTimerManager().SetTimer(BriefingTimer, this,
		&ARANDMission_Consultation::SendBriefing, 0.8f, /*bLoop=*/false);

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Thandi — the official handing out the tender. Afrikaans, per the GDD.
	const TSubclassOf<ARANDCharacter_NPC> ThandiToSpawn =
		ThandiClass ? ThandiClass : ARANDCharacter_NPC::StaticClass();
	Thandi = World->SpawnActor<ARANDCharacter_NPC>(ThandiToSpawn,
		GetActorTransform().TransformPosition(ThandiSpawnOffset), FRotator::ZeroRotator, Params);
	if (Thandi)
	{
		Thandi->NPCName = TEXT("Thandi");
		Thandi->NPCType = ENPCType::Official;
		Thandi->LanguageGroup = ELanguageGroup::Afrikaans;
		SetupThandiDialogue();
		if (URANDDialogueComponent* Dialogue = Thandi->GetDialogueComponent())
		{
			Dialogue->OnDialogueEnded.AddDynamic(this, &ARANDMission_Consultation::HandleThandiDialogueEnded);
		}
	}

	// Tender desk — inert until André has spoken to Thandi.
	const TSubclassOf<ARANDTenderDesk> DeskToSpawn =
		DeskClass ? DeskClass : ARANDTenderDesk::StaticClass();
	Desk = World->SpawnActor<ARANDTenderDesk>(DeskToSpawn,
		GetActorTransform().TransformPosition(DeskSpawnOffset), FRotator::ZeroRotator, Params);
	if (Desk)
	{
		Desk->bAcceptingSubmission = false;
		Desk->OnTenderSubmitted.AddDynamic(this, &ARANDMission_Consultation::HandleTenderSubmitted);
	}
}

void ARANDMission_Consultation::SetupThandiDialogue()
{
	URANDDialogueComponent* Dialogue = Thandi ? Thandi->GetDialogueComponent() : nullptr;
	if (!Dialogue)
	{
		return;
	}

	auto MakeLine = [](const FString& Speaker, const FText& Text, const FText& ChoiceText, int32 Next)
	{
		FRANDDialogueLine Line;
		Line.Speaker = Speaker;
		Line.Line = Text;
		FRANDDialogueChoice Choice;
		Choice.ChoiceText = ChoiceText;
		Choice.NextLineIndex = Next;
		Line.Choices.Add(Choice);
		return Line;
	};

	Dialogue->DialogueLines.Empty();
	Dialogue->DialogueLines.Add(MakeLine(TEXT("Thandi"),
		NSLOCTEXT("RANDMission", "Thandi_1", "Meneer Venter. Welkom, sit gerus."),
		NSLOCTEXT("RANDMission", "Thandi_1c", "Thank you."), 1));
	Dialogue->DialogueLines.Add(MakeLine(TEXT("Thandi"),
		NSLOCTEXT("RANDMission", "Thandi_2", "Ek het 'n tender wat jou sal interesseer. Vul net die vorm in by die lessenaar."),
		NSLOCTEXT("RANDMission", "Thandi_2c", "I'll take care of it."), -1));
}

void ARANDMission_Consultation::HandleOfficeOverlap(UPrimitiveComponent* /*OverlappedComp*/,
	AActor* OtherActor, UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/, const FHitResult& /*Sweep*/)
{
	if (bReachedOffice || !Cast<ARANDCharacter>(OtherActor))
	{
		return;
	}

	bReachedOffice = true;
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this))
	{
		Missions->CompleteObjective(MissionID, 0);
	}
}

void ARANDMission_Consultation::HandleThandiDialogueEnded()
{
	if (bTalkedToThandi)
	{
		return;
	}

	bTalkedToThandi = true;

	// Now André can submit the tender.
	if (Desk)
	{
		Desk->bAcceptingSubmission = true;
	}

	if (URANDMissionManager* Missions = URANDMissionManager::Get(this))
	{
		Missions->CompleteObjective(MissionID, 1);
	}
}

void ARANDMission_Consultation::HandleTenderSubmitted(AActor* /*Interactor*/)
{
	if (bSubmittedTender)
	{
		return;
	}

	bSubmittedTender = true;
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this))
	{
		Missions->CompleteObjective(MissionID, 2);
	}

	// Thandi follows up with her offer.
	OfferBribe();
}

void ARANDMission_Consultation::SendBriefing()
{
	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this))
	{
		Phone->ReceiveMessage(TEXT("Sipho"),
			NSLOCTEXT("RANDMission", "Brief_Consultation",
				"Andre - the Marshalltown tender is ready. Get to the office and see Thandi."));
	}
}

void ARANDMission_Consultation::OfferBribe()
{
	bBribeOffered = true;

	// Thandi's offer arrives by phone with accept/decline replies that route
	// back through HandlePhoneOption.
	URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this);
	if (!Phone)
	{
		return;
	}

	Phone->OnMessageOptionSelected.AddUniqueDynamic(this, &ARANDMission_Consultation::HandlePhoneOption);

	FRANDMessageOption Accept;
	Accept.Label = NSLOCTEXT("RANDMission", "Bribe_Accept", "Accept (R85,000)");
	Accept.ActionId = FName(TEXT("bribe_accept"));

	FRANDMessageOption Decline;
	Decline.Label = NSLOCTEXT("RANDMission", "Bribe_Decline", "Decline");
	Decline.ActionId = FName(TEXT("bribe_decline"));

	Phone->ReceiveMessageWithOptions(TEXT("Thandi"),
		NSLOCTEXT("RANDMission", "Bribe_Offer",
			"Push this tender through and there's R85,000 in it for you. We never had this chat."),
		{ Accept, Decline });
}

void ARANDMission_Consultation::HandlePhoneOption(FName ActionId)
{
	if (ActionId == FName(TEXT("bribe_accept")))
	{
		AcceptBribe();
	}
	else if (ActionId == FName(TEXT("bribe_decline")))
	{
		DeclineBribe();
	}
}

void ARANDMission_Consultation::AcceptBribe()
{
	if (bResolved || !bBribeOffered)
	{
		return;
	}
	bResolved = true;

	if (ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		Player->bAcceptedBribe = true;
		if (URANDEconomyComponent* Econ = Player->GetEconomyComponent())
		{
			Econ->AddFunds(85000.0f, TEXT("Tender Bribe"));
		}
		if (UWantedComponent* Wanted = Player->GetWantedComponent())
		{
			Wanted->AddHeat(EAgency::Hawks, 20.0f);
		}
	}

	if (URANDMissionManager* Missions = URANDMissionManager::Get(this))
	{
		Missions->CompleteObjective(MissionID, 3);
	}
}

void ARANDMission_Consultation::DeclineBribe()
{
	if (bResolved || !bBribeOffered)
	{
		return;
	}
	bResolved = true;

	if (ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		Player->bAcceptedBribe = false;
	}

	if (URANDMissionManager* Missions = URANDMissionManager::Get(this))
	{
		Missions->CompleteObjective(MissionID, 3);
	}
}
