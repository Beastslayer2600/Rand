// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RANDLanguageTypes.h"
#include "RANDDialogueBank.generated.h"

/**
 * URANDDialogueBank — process-wide singleton holding all colloquial NPC
 * reaction lines (per language, per situation) and the district language
 * profiles. Pure static reference data, built once on first access.
 *
 * Usage:
 *   URANDDialogueBank* Bank = URANDDialogueBank::Get();
 *   ELanguageGroup Lang = Bank->PickLanguageForDistrict(EDistrict::Hillbrow);
 *   FText Line = Bank->GetRandomLine(Lang, ESituationType::Greeting);
 */
UCLASS()
class RAND_API URANDDialogueBank : public UObject
{
	GENERATED_BODY()

public:
	/** Lazily creates the singleton (root-held so it survives GC) and returns it. */
	static URANDDialogueBank* Get();

	/** All lines for a language, grouped by situation. */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	const FLanguageDialogueSet& GetDialogueSet(ELanguageGroup Language) const;

	/** A random line for the given language + situation, or empty text if none exist. */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FText GetRandomLine(ELanguageGroup Language, ESituationType Situation) const;

	/** The language profile for a district. */
	UFUNCTION(BlueprintPure, Category = "Language")
	const FDistrictLanguageProfile& GetDistrictProfile(EDistrict District) const;

	/** Weighted-random language for NPCs spawning in a district. */
	UFUNCTION(BlueprintPure, Category = "Language")
	ELanguageGroup PickLanguageForDistrict(EDistrict District) const;

private:
	UPROPERTY()
	TMap<ELanguageGroup, FLanguageDialogueSet> DialogueSets;

	UPROPERTY()
	TMap<EDistrict, FDistrictLanguageProfile> DistrictProfiles;

	/** Populates DialogueSets with the colloquial reaction lines. */
	void BuildDialogue();

	/** Populates DistrictProfiles from census-derived weights. */
	void BuildDistrictProfiles();
};
