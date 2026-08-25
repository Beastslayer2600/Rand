// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RANDReputationComponent.generated.h"

UENUM(BlueprintType)
enum class ERANDContact : uint8
{
	Sipho		UMETA(DisplayName = "Sipho Dlamini"),
	Thandi		UMETA(DisplayName = "Thandi Mokoena"),
	RankMarshal	UMETA(DisplayName = "Park Station Marshal"),
	Lawyer		UMETA(DisplayName = "Advocate Naidoo"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnContactStandingChanged, ERANDContact, Contact, float, NewStanding);

UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDReputationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDReputationComponent();

	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void AddStanding(ERANDContact Contact, float Delta);

	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void SetStanding(ERANDContact Contact, float Value);

	UFUNCTION(BlueprintPure, Category = "Reputation")
	float GetStanding(ERANDContact Contact) const;

	UFUNCTION(BlueprintPure, Category = "Reputation")
	FString GetContactName(ERANDContact Contact) const;

	UPROPERTY(BlueprintAssignable, Category = "Reputation")
	FOnContactStandingChanged OnStandingChanged;

	void LoadStandings(const TArray<float>& InStandings);
	void CopyStandings(TArray<float>& OutStandings) const;

private:
	static constexpr int32 ContactCount = 4;
	float Standing[ContactCount] = {};

	static int32 IndexOf(ERANDContact Contact) { return static_cast<int32>(Contact); }
};
