// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDCharacter_NPC.h"
#include "RANDCharacter.h"
#include "DialogueComponent.h"
#include "HealthComponent.h"
#include "RANDDialogueBank.h"
#include "RANDNPCAIController.h"
#include "WantedComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

ARANDCharacter_NPC::ARANDCharacter_NPC()
{
#if WITH_EDITOR
	PrimaryActorTick.bCanEverTick = true;
#else
	PrimaryActorTick.bCanEverTick = false;
#endif

	DialogueComponent = CreateDefaultSubobject<URANDDialogueComponent>(TEXT("DialogueComponent"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	AIControllerClass = ARANDNPCAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> PlaceholderMesh(
		TEXT("/MoverExamples/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	if (PlaceholderMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(PlaceholderMesh.Object);
	}
}

void ARANDCharacter_NPC::BeginPlay()
{
	Super::BeginPlay();
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &ARANDCharacter_NPC::HandleDeath);
	}
}

void ARANDCharacter_NPC::HandleDeath()
{
	SetLifeSpan(1.5f);
}

#if WITH_EDITOR
void ARANDCharacter_NPC::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const FString DisplayName = NPCName.IsEmpty() ? FString(TEXT("(unnamed)")) : NPCName;
	const FString LangName = StaticEnum<ELanguageGroup>()
		->GetDisplayNameTextByValue(static_cast<int64>(LanguageGroup)).ToString();
	DrawDebugString(GetWorld(), GetActorLocation() + FVector(0.0f, 0.0f, 120.0f),
		FString::Printf(TEXT("%s\n%s"), *DisplayName, *LangName),
		nullptr, FColor::White, 0.0f, true);
}
#endif

FText ARANDCharacter_NPC::GetInteractionPrompt_Implementation() const
{
	return FText::Format(NSLOCTEXT("RANDNPC", "TalkTo", "Talk to {0}"), FText::FromString(NPCName));
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
	return DialogueComponent && DialogueComponent->HasDialogue() && !DialogueComponent->IsDialogueActive();
}

void ARANDCharacter_NPC::AssignLanguageFromDistrict(EDistrict District)
{
	LanguageGroup = URANDDialogueBank::Get()->PickLanguageForDistrict(District);
}

FText ARANDCharacter_NPC::GetReactionLine(ESituationType Situation) const
{
	return URANDDialogueBank::Get()->GetRandomLine(LanguageGroup, Situation);
}

bool ARANDCharacter_NPC::ReportCrime(EAgency Agency, float HeatAmount)
{
	if (HeatAmount <= 0.0f) return false;
	ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player) return false;
	const float DistSq = FVector::DistSquared(GetActorLocation(), Player->GetActorLocation());
	if (DistSq > FMath::Square(WitnessRadius)) return false;
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
	case ENPCType::Police:   return EAgency::SAPS;
	case ENPCType::Official: return EAgency::Hawks;
	case ENPCType::Criminal: return EAgency::Rivals;
	case ENPCType::Contact:  return EAgency::Rivals;
	default:                 return EAgency::SAPS;
	}
}
