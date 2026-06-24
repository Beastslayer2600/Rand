// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDCharacter_NPC.h"
#include "RANDCharacter.h"
#include "DialogueComponent.h"
#include "WantedComponent.h"

#include "Kismet/GameplayStatics.h"

ARANDCharacter_NPC::ARANDCharacter_NPC()
{
	PrimaryActorTick.bCanEverTick = false;

	DialogueComponent = CreateDefaultSubobject<URANDDialogueComponent>(TEXT("DialogueComponent"));
}

// --- IInteractable ----------------------------------------------------------

FText ARANDCharacter_NPC::GetInteractionPrompt_Implementation() const
{
	return FText::Format(
		NSLOCTEXT("RANDNPC", "TalkTo", "Talk to {0}"), FText::FromString(NPCName));
}

void ARANDCharacter_NPC::OnInteract_Implementation(AActor* /*Interactor*/)
{
	if (DialogueComponent)
	{
		DialogueComponent->StartDialogue();
	}
}

bool ARANDCharacter_NPC::CanInteract_Implementation(const AActor* /*Interactor*/) const
{
	// Only offer interaction when there's a conversation to have and we're not
	// already mid-dialogue with this NPC.
	return DialogueComponent && DialogueComponent->HasDialogue()
		&& !DialogueComponent->IsDialogueActive();
}

// --- Witnessing -------------------------------------------------------------

bool ARANDCharacter_NPC::ReportCrime(EAgency Agency, float HeatAmount)
{
	if (HeatAmount <= 0.0f)
	{
		return false;
	}

	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player)
	{
		return false;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), Player->GetActorLocation());
	if (DistSq > FMath::Square(WitnessRadius))
	{
		return false; // Player out of sight/earshot.
	}

	if (UWantedComponent* Heat = Player->GetWantedComponent())
	{
		Heat->AddHeat(Agency, HeatAmount);
		return true;
	}
	return false;
}

bool ARANDCharacter_NPC::WitnessCrime(float HeatAmount)
{
	return ReportCrime(GetDefaultReportingAgency(), HeatAmount);
}

EAgency ARANDCharacter_NPC::GetDefaultReportingAgency() const
{
	switch (NPCType)
	{
	case ENPCType::Police:   return EAgency::SAPS;   // Street-level law enforcement.
	case ENPCType::Official: return EAgency::Hawks;  // Officials escalate to priority crime.
	case ENPCType::Criminal: return EAgency::Rivals; // Rival factions track their own scores.
	case ENPCType::Contact:  return EAgency::Rivals; // Underworld contacts answer to factions.
	case ENPCType::Civilian:
	default:                 return EAgency::SAPS;   // Civilians call the police.
	}
}
