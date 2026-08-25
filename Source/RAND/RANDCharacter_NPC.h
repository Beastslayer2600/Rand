// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IInteractable.h"
#include "WantedComponent.h"
#include "RANDLanguageTypes.h"
#include "RANDCharacter_NPC.generated.h"

class URANDDialogueComponent;
class UHealthComponent;

UENUM(BlueprintType)
enum class ENPCType : uint8
{
	Civilian	UMETA(DisplayName = "Civilian"),
	Official	UMETA(DisplayName = "Official"),
	Criminal	UMETA(DisplayName = "Criminal"),
	Police		UMETA(DisplayName = "Police"),
	Contact		UMETA(DisplayName = "Contact"),
};

UCLASS()
class RAND_API ARANDCharacter_NPC : public ACharacter, public IInteractable
{
	GENERATED_BODY()

public:
	ARANDCharacter_NPC();

#if WITH_EDITOR
	virtual void Tick(float DeltaSeconds) override;
#endif

	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(const AActor* Interactor) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FString NPCName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	ENPCType NPCType = ENPCType::Civilian;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	ELanguageGroup LanguageGroup = ELanguageGroup::English;

	UFUNCTION(BlueprintCallable, Category = "NPC|Language")
	void AssignLanguageFromDistrict(EDistrict District);

	UFUNCTION(BlueprintCallable, Category = "NPC|Language")
	FText GetReactionLine(ESituationType Situation) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Witness")
	float WitnessRadius = 1500.0f;

	UFUNCTION(BlueprintCallable, Category = "NPC|Witness")
	bool ReportCrime(EAgency Agency, float HeatAmount);

	UFUNCTION(BlueprintCallable, Category = "NPC|Witness")
	bool WitnessCrime(float HeatAmount);

	UFUNCTION(BlueprintPure, Category = "NPC|Witness")
	EAgency GetDefaultReportingAgency() const;

	UFUNCTION(BlueprintPure, Category = "NPC")
	URANDDialogueComponent* GetDialogueComponent() const { return DialogueComponent; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URANDDialogueComponent> DialogueComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;

	UFUNCTION()
	void HandleDeath();
};
