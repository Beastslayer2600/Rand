// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDDistrictVolume.h"
#include "RANDCharacter_NPC.h"

#include "Components/BrushComponent.h"
#include "EngineUtils.h"

ARANDDistrictVolume::ARANDDistrictVolume()
{
	// Query-only overlap so the brush can be tested against NPC positions
	// without participating in physics.
	if (UBrushComponent* Brush = GetBrushComponent())
	{
		Brush->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Brush->SetCollisionResponseToAllChannels(ECR_Overlap);
	}
}

void ARANDDistrictVolume::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Assign this district's language to every NPC already standing inside the
	// volume. EncompassesPoint is a deterministic geometric test, so it doesn't
	// depend on physics-overlap registration timing at load.
	for (TActorIterator<ARANDCharacter_NPC> It(World); It; ++It)
	{
		ARANDCharacter_NPC* NPC = *It;
		if (NPC && EncompassesPoint(NPC->GetActorLocation()))
		{
			NPC->AssignLanguageFromDistrict(District);
		}
	}
}
