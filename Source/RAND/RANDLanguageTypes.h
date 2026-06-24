// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RANDLanguageTypes.generated.h"

/**
 * The 11 official languages of South Africa. Assigned per-NPC and used to pick
 * the right colloquial reaction lines from the dialogue bank.
 */
UENUM(BlueprintType)
enum class ELanguageGroup : uint8
{
	Zulu		UMETA(DisplayName = "isiZulu"),
	Xhosa		UMETA(DisplayName = "isiXhosa"),
	Afrikaans	UMETA(DisplayName = "Afrikaans"),
	Sepedi		UMETA(DisplayName = "Sepedi (Northern Sotho)"),
	Tswana		UMETA(DisplayName = "Setswana"),
	Sotho		UMETA(DisplayName = "Sesotho (Southern Sotho)"),
	Tsonga		UMETA(DisplayName = "Xitsonga"),
	Swati		UMETA(DisplayName = "siSwati"),
	Venda		UMETA(DisplayName = "Tshivenda"),
	Ndebele		UMETA(DisplayName = "isiNdebele"),
	English		UMETA(DisplayName = "English"),
};

/**
 * Playable districts across the Johannesburg CBD and Pretoria. Each has a
 * language profile reflecting real census-derived home-language distribution.
 */
UENUM(BlueprintType)
enum class EDistrict : uint8
{
	// Johannesburg
	Hillbrow		UMETA(DisplayName = "Hillbrow"),
	MarshallTown	UMETA(DisplayName = "Marshalltown"),
	ParkStation		UMETA(DisplayName = "Park Station"),
	Maboneng		UMETA(DisplayName = "Maboneng"),
	// Pretoria
	PretoriaCBD		UMETA(DisplayName = "Pretoria CBD"),
	Arcadia			UMETA(DisplayName = "Arcadia"),
	Sunnyside		UMETA(DisplayName = "Sunnyside"),
	Hatfield		UMETA(DisplayName = "Hatfield"),
	Centurion		UMETA(DisplayName = "Centurion"),
};

/** Situations that trigger an NPC reaction line. */
UENUM(BlueprintType)
enum class ESituationType : uint8
{
	Greeting		UMETA(DisplayName = "Greeting"),
	CrimeWitnessed	UMETA(DisplayName = "Crime Witnessed"),
	Hostile			UMETA(DisplayName = "Hostile"),
};

/**
 * FDistrictLanguageProfile — the home-language mix of a district. Weights are
 * expressed as fractions that sum to 1.0 and drive weighted-random language
 * assignment when NPCs spawn in that district.
 */
USTRUCT(BlueprintType)
struct FDistrictLanguageProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Language")
	TMap<ELanguageGroup, float> LanguageWeights;

	/**
	 * Weighted-random language pick from this profile. Tolerates weights that
	 * don't sum to exactly 1.0 (normalises against the actual total). Falls back
	 * to English if the profile is empty.
	 */
	ELanguageGroup PickWeightedLanguage() const;
};

/**
 * FLanguageDialogueSet — all reaction lines for a single language, grouped by
 * situation. NPCs pick a random entry from the relevant array.
 */
USTRUCT(BlueprintType)
struct FLanguageDialogueSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FText> Greetings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FText> CrimeWitnessed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FText> Hostile;

	/** Returns the line array for a situation (empty array if none). */
	const TArray<FText>& GetLines(ESituationType Situation) const;
};
