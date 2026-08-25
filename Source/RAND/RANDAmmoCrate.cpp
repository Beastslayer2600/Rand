// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDAmmoCrate.h"
#include "RANDCharacter.h"
#include "EconomyComponent.h"
#include "RANDCombatComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ARANDAmmoCrate::ARANDAmmoCrate()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Mesh->SetStaticMesh(Cube.Object);
	Mesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.4f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	Mesh->SetGenerateOverlapEvents(true);
}

FText ARANDAmmoCrate::GetInteractionPrompt_Implementation() const
{
	return FText::FromString(FString::Printf(TEXT("Buy ammo (%d)  R%.0f"), Rounds, Price));
}

bool ARANDAmmoCrate::CanInteract_Implementation(const AActor*) const { return true; }

void ARANDAmmoCrate::OnInteract_Implementation(AActor* Interactor)
{
	ARANDCharacter* Player = Cast<ARANDCharacter>(Interactor);
	if (!Player || !Player->GetEconomyComponent() || !Player->GetCombatComponent()) return;
	if (!Player->GetCombatComponent()->IsArmed()) return;
	if (!Player->GetEconomyComponent()->DeductFunds(Price, TEXT("Street ammo"))) return;
	Player->GetCombatComponent()->AddReserve(Rounds);
}
