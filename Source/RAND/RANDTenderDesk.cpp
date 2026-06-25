// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDTenderDesk.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ARANDTenderDesk::ARANDTenderDesk()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeskMesh"));
	SetRootComponent(Mesh);

	// Placeholder cube, scaled to a desk-ish slab.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
	Mesh->SetRelativeScale3D(FVector(1.4f, 0.7f, 0.8f));

	// Query-overlap so the player's interaction sphere reliably detects the
	// desk (a blocking response would register as a hit, not an overlap).
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	Mesh->SetGenerateOverlapEvents(true);
}

FText ARANDTenderDesk::GetInteractionPrompt_Implementation() const
{
	return NSLOCTEXT("RANDTenderDesk", "SubmitTender", "Submit Tender");
}

void ARANDTenderDesk::OnInteract_Implementation(AActor* Interactor)
{
	if (bSubmitted || !bAcceptingSubmission)
	{
		return;
	}

	bSubmitted = true;
	OnTenderSubmitted.Broadcast(Interactor);
}

bool ARANDTenderDesk::CanInteract_Implementation(const AActor* /*Interactor*/) const
{
	return bAcceptingSubmission && !bSubmitted;
}
