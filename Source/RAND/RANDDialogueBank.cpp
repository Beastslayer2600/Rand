// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDDialogueBank.h"

// The dialogue below contains non-ASCII letters (e.g. ê, š). This file is
// saved as UTF-8 with a BOM so MSVC reads those characters correctly; keep the
// BOM if you re-save it.

// --- FDistrictLanguageProfile ----------------------------------------------

ELanguageGroup FDistrictLanguageProfile::PickWeightedLanguage() const
{
	float Total = 0.0f;
	for (const TPair<ELanguageGroup, float>& Pair : LanguageWeights)
	{
		Total += FMath::Max(0.0f, Pair.Value);
	}

	if (Total <= 0.0f)
	{
		return ELanguageGroup::English; // Empty/degenerate profile — safe default.
	}

	float Roll = FMath::FRand() * Total;
	for (const TPair<ELanguageGroup, float>& Pair : LanguageWeights)
	{
		Roll -= FMath::Max(0.0f, Pair.Value);
		if (Roll <= 0.0f)
		{
			return Pair.Key;
		}
	}

	// Floating-point slack: return the last positive-weight entry.
	for (const TPair<ELanguageGroup, float>& Pair : LanguageWeights)
	{
		if (Pair.Value > 0.0f)
		{
			return Pair.Key;
		}
	}
	return ELanguageGroup::English;
}

// --- FLanguageDialogueSet ---------------------------------------------------

const TArray<FText>& FLanguageDialogueSet::GetLines(ESituationType Situation) const
{
	switch (Situation)
	{
	case ESituationType::CrimeWitnessed: return CrimeWitnessed;
	case ESituationType::Hostile:        return Hostile;
	case ESituationType::Greeting:
	default:                             return Greetings;
	}
}

// --- URANDDialogueBank ------------------------------------------------------

URANDDialogueBank* URANDDialogueBank::Get()
{
	static URANDDialogueBank* Instance = nullptr;
	if (!Instance)
	{
		Instance = NewObject<URANDDialogueBank>(GetTransientPackage(), URANDDialogueBank::StaticClass());
		Instance->AddToRoot(); // Reference data — keep alive for the whole process.
		Instance->BuildDialogue();
		Instance->BuildDistrictProfiles();
	}
	return Instance;
}

const FLanguageDialogueSet& URANDDialogueBank::GetDialogueSet(ELanguageGroup Language) const
{
	static const FLanguageDialogueSet Empty;
	const FLanguageDialogueSet* Set = DialogueSets.Find(Language);
	return Set ? *Set : Empty;
}

FText URANDDialogueBank::GetRandomLine(ELanguageGroup Language, ESituationType Situation) const
{
	const FLanguageDialogueSet& Set = GetDialogueSet(Language);
	const TArray<FText>& Lines = Set.GetLines(Situation);
	if (Lines.Num() == 0)
	{
		return FText::GetEmpty();
	}
	return Lines[FMath::RandRange(0, Lines.Num() - 1)];
}

const FDistrictLanguageProfile& URANDDialogueBank::GetDistrictProfile(EDistrict District) const
{
	static const FDistrictLanguageProfile Empty;
	const FDistrictLanguageProfile* Profile = DistrictProfiles.Find(District);
	return Profile ? *Profile : Empty;
}

ELanguageGroup URANDDialogueBank::PickLanguageForDistrict(EDistrict District) const
{
	if (const FDistrictLanguageProfile* Profile = DistrictProfiles.Find(District))
	{
		return Profile->PickWeightedLanguage();
	}
	return ELanguageGroup::English;
}

// --- Data ------------------------------------------------------------------

void URANDDialogueBank::BuildDialogue()
{
	auto Line = [](const TCHAR* Text) { return FText::FromString(Text); };

	auto MakeSet = [](TArray<FText> Greet, TArray<FText> Crime, TArray<FText> Hostile)
	{
		FLanguageDialogueSet Set;
		Set.Greetings = MoveTemp(Greet);
		Set.CrimeWitnessed = MoveTemp(Crime);
		Set.Hostile = MoveTemp(Hostile);
		return Set;
	};

	DialogueSets.Add(ELanguageGroup::Zulu, MakeSet(
		{ Line(TEXT("Sawubona")), Line(TEXT("Yebo")) },
		{ Line(TEXT("Haibo!")), Line(TEXT("Yini lena!")), Line(TEXT("Shaya amaphoyisa!")) },
		{ Line(TEXT("Hamba!")), Line(TEXT("Suka la!")), Line(TEXT("Ungangithinti!")) }));

	DialogueSets.Add(ELanguageGroup::Xhosa, MakeSet(
		{ Line(TEXT("Molo")), Line(TEXT("Enkosi")) },
		{ Line(TEXT("Hayi bo!")), Line(TEXT("Nceda!")), Line(TEXT("Amapolisa!")) },
		{ Line(TEXT("Hamba!")), Line(TEXT("Sukani apha!")) }));

	DialogueSets.Add(ELanguageGroup::Afrikaans, MakeSet(
		{ Line(TEXT("Howzit")), Line(TEXT("Lekker nê")) },
		{ Line(TEXT("Eish nee man!")), Line(TEXT("Bel die polisie!")), Line(TEXT("Los hom!")) },
		{ Line(TEXT("Los my uit!")), Line(TEXT("Voertsek!")), Line(TEXT("Trap!")) }));

	DialogueSets.Add(ELanguageGroup::Sepedi, MakeSet(
		{ Line(TEXT("Dumela")), Line(TEXT("Ee")) },
		{ Line(TEXT("Heish!")), Line(TEXT("Bitša maphodisa!")) },
		{ Line(TEXT("Tšwa moo!")), Line(TEXT("Ntlogela!")) }));

	DialogueSets.Add(ELanguageGroup::Tswana, MakeSet(
		{ Line(TEXT("Dumela")), Line(TEXT("Ke a leboga")) },
		{ Line(TEXT("Heish!")), Line(TEXT("Biletsa mapodisi!")) },
		{ Line(TEXT("Tsamaya!")), Line(TEXT("Ntlogele!")) }));

	DialogueSets.Add(ELanguageGroup::Sotho, MakeSet(
		{ Line(TEXT("Dumela")), Line(TEXT("Eya")) },
		{ Line(TEXT("Heish!")), Line(TEXT("Bitsa mapolesa!")) },
		{ Line(TEXT("Tsamaya!")), Line(TEXT("Ntlogela!")) }));

	DialogueSets.Add(ELanguageGroup::Tsonga, MakeSet(
		{ Line(TEXT("Avuxeni")), Line(TEXT("Inkomu")) },
		{ Line(TEXT("Hawa!")), Line(TEXT("Vitana maphorisa!")) },
		{ Line(TEXT("Hamba!")), Line(TEXT("Ndzi siya!")) }));

	DialogueSets.Add(ELanguageGroup::Swati, MakeSet(
		{ Line(TEXT("Sawubona")), Line(TEXT("Yebo")) },
		{ Line(TEXT("Haibo!")), Line(TEXT("Bita emaPhoyisa!")) },
		{ Line(TEXT("Hamba!")), Line(TEXT("Ngisuke!")) }));

	DialogueSets.Add(ELanguageGroup::Venda, MakeSet(
		{ Line(TEXT("Ndaa")), Line(TEXT("Aa")) },
		{ Line(TEXT("Ndi mini!")), Line(TEXT("Vhidzani mapholisa!")) },
		{ Line(TEXT("Bva hone!")), Line(TEXT("Ntshandukela!")) }));

	DialogueSets.Add(ELanguageGroup::Ndebele, MakeSet(
		{ Line(TEXT("Lotjhani")), Line(TEXT("Yebo")) },
		{ Line(TEXT("Hayibo!")), Line(TEXT("Biza amaphoyisa!")) },
		{ Line(TEXT("Hamba!")), Line(TEXT("Ngisuke la!")) }));

	DialogueSets.Add(ELanguageGroup::English, MakeSet(
		{ Line(TEXT("Howzit")), Line(TEXT("Sharp sharp")) },
		{ Line(TEXT("Eish what the hell!")), Line(TEXT("Call the police!")) },
		{ Line(TEXT("Wena! Back off!")), Line(TEXT("Don't play with me!")) }));
}

void URANDDialogueBank::BuildDistrictProfiles()
{
	auto MakeProfile = [](TMap<ELanguageGroup, float> Weights)
	{
		FDistrictLanguageProfile Profile;
		Profile.LanguageWeights = MoveTemp(Weights);
		return Profile;
	};

	// --- Johannesburg ------------------------------------------------------

	DistrictProfiles.Add(EDistrict::Hillbrow, MakeProfile({
		{ ELanguageGroup::Zulu, 0.28f }, { ELanguageGroup::Sotho, 0.18f },
		{ ELanguageGroup::Xhosa, 0.15f }, { ELanguageGroup::English, 0.12f },
		{ ELanguageGroup::Tswana, 0.08f }, { ELanguageGroup::Sepedi, 0.07f },
		{ ELanguageGroup::Tsonga, 0.05f }, { ELanguageGroup::Ndebele, 0.03f },
		{ ELanguageGroup::Venda, 0.02f }, { ELanguageGroup::Swati, 0.01f },
		{ ELanguageGroup::Afrikaans, 0.01f } }));

	DistrictProfiles.Add(EDistrict::MarshallTown, MakeProfile({
		{ ELanguageGroup::English, 0.28f }, { ELanguageGroup::Zulu, 0.22f },
		{ ELanguageGroup::Sotho, 0.15f }, { ELanguageGroup::Tswana, 0.10f },
		{ ELanguageGroup::Afrikaans, 0.08f }, { ELanguageGroup::Xhosa, 0.07f },
		{ ELanguageGroup::Sepedi, 0.05f }, { ELanguageGroup::Tsonga, 0.03f },
		{ ELanguageGroup::Venda, 0.01f }, { ELanguageGroup::Ndebele, 0.01f },
		{ ELanguageGroup::Swati, 0.00f } }));

	DistrictProfiles.Add(EDistrict::ParkStation, MakeProfile({
		{ ELanguageGroup::Zulu, 0.25f }, { ELanguageGroup::Sotho, 0.18f },
		{ ELanguageGroup::Tsonga, 0.12f }, { ELanguageGroup::Tswana, 0.10f },
		{ ELanguageGroup::English, 0.10f }, { ELanguageGroup::Xhosa, 0.08f },
		{ ELanguageGroup::Sepedi, 0.07f }, { ELanguageGroup::Ndebele, 0.04f },
		{ ELanguageGroup::Venda, 0.03f }, { ELanguageGroup::Afrikaans, 0.02f },
		{ ELanguageGroup::Swati, 0.01f } }));

	DistrictProfiles.Add(EDistrict::Maboneng, MakeProfile({
		{ ELanguageGroup::English, 0.35f }, { ELanguageGroup::Zulu, 0.20f },
		{ ELanguageGroup::Sotho, 0.15f }, { ELanguageGroup::Xhosa, 0.10f },
		{ ELanguageGroup::Tswana, 0.08f }, { ELanguageGroup::Afrikaans, 0.05f },
		{ ELanguageGroup::Sepedi, 0.04f }, { ELanguageGroup::Tsonga, 0.02f },
		{ ELanguageGroup::Venda, 0.01f }, { ELanguageGroup::Ndebele, 0.00f },
		{ ELanguageGroup::Swati, 0.00f } }));

	// --- Pretoria ----------------------------------------------------------

	DistrictProfiles.Add(EDistrict::PretoriaCBD, MakeProfile({
		{ ELanguageGroup::Afrikaans, 0.28f }, { ELanguageGroup::Sepedi, 0.22f },
		{ ELanguageGroup::Tswana, 0.20f }, { ELanguageGroup::English, 0.15f },
		{ ELanguageGroup::Zulu, 0.06f }, { ELanguageGroup::Sotho, 0.04f },
		{ ELanguageGroup::Xhosa, 0.02f }, { ELanguageGroup::Tsonga, 0.02f },
		{ ELanguageGroup::Venda, 0.01f }, { ELanguageGroup::Ndebele, 0.00f },
		{ ELanguageGroup::Swati, 0.00f } }));

	DistrictProfiles.Add(EDistrict::Arcadia, MakeProfile({
		{ ELanguageGroup::English, 0.40f }, { ELanguageGroup::Afrikaans, 0.25f },
		{ ELanguageGroup::Tswana, 0.12f }, { ELanguageGroup::Sepedi, 0.10f },
		{ ELanguageGroup::Zulu, 0.05f }, { ELanguageGroup::Sotho, 0.04f },
		{ ELanguageGroup::Xhosa, 0.02f }, { ELanguageGroup::Tsonga, 0.01f },
		{ ELanguageGroup::Venda, 0.01f }, { ELanguageGroup::Ndebele, 0.00f },
		{ ELanguageGroup::Swati, 0.00f } }));

	DistrictProfiles.Add(EDistrict::Sunnyside, MakeProfile({
		{ ELanguageGroup::Sepedi, 0.20f }, { ELanguageGroup::Tswana, 0.18f },
		{ ELanguageGroup::English, 0.17f }, { ELanguageGroup::Zulu, 0.14f },
		{ ELanguageGroup::Afrikaans, 0.10f }, { ELanguageGroup::Sotho, 0.08f },
		{ ELanguageGroup::Xhosa, 0.06f }, { ELanguageGroup::Tsonga, 0.04f },
		{ ELanguageGroup::Venda, 0.02f }, { ELanguageGroup::Ndebele, 0.01f },
		{ ELanguageGroup::Swati, 0.00f } }));

	DistrictProfiles.Add(EDistrict::Hatfield, MakeProfile({
		{ ELanguageGroup::English, 0.38f }, { ELanguageGroup::Afrikaans, 0.20f },
		{ ELanguageGroup::Tswana, 0.15f }, { ELanguageGroup::Sepedi, 0.12f },
		{ ELanguageGroup::Zulu, 0.06f }, { ELanguageGroup::Sotho, 0.05f },
		{ ELanguageGroup::Xhosa, 0.03f }, { ELanguageGroup::Tsonga, 0.01f },
		{ ELanguageGroup::Venda, 0.00f }, { ELanguageGroup::Ndebele, 0.00f },
		{ ELanguageGroup::Swati, 0.00f } }));

	DistrictProfiles.Add(EDistrict::Centurion, MakeProfile({
		{ ELanguageGroup::Afrikaans, 0.35f }, { ELanguageGroup::Tswana, 0.20f },
		{ ELanguageGroup::English, 0.15f }, { ELanguageGroup::Zulu, 0.12f },
		{ ELanguageGroup::Sepedi, 0.08f }, { ELanguageGroup::Sotho, 0.06f },
		{ ELanguageGroup::Tsonga, 0.02f }, { ELanguageGroup::Xhosa, 0.01f },
		{ ELanguageGroup::Venda, 0.01f }, { ELanguageGroup::Ndebele, 0.00f },
		{ ELanguageGroup::Swati, 0.00f } }));
}
