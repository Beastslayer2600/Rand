// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RANDGameMode.generated.h"

/**
 * ARANDGameMode — default rules for RAND. Spawns André (ARANDCharacter) as
 * the player pawn. Single-player; expands later with save/load and the
 * economy/heat systems described in the GDD.
 */
UCLASS()
class RAND_API ARANDGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARANDGameMode();
};
