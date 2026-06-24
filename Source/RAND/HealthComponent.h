// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

/**
 * UHealthComponent — generic health/damage layer for André (and any actor that
 * needs HP: civilians, enemies, vehicles). Hooks into AActor::TakeDamage so
 * standard UE damage events (point damage, radial damage, etc.) work without
 * custom glue.
 *
 * Death is signalled by the OnDeath delegate; the component does NOT destroy
 * the owner — game-mode logic or an animation layer handles the response.
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	// --- Configuration ------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;

	// --- Queries ------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bDead; }

	// --- Mutation -----------------------------------------------------------

	/** Restore HP, clamped to MaxHealth. Ignored if the owner is dead. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	// --- Delegates ----------------------------------------------------------

	/** Fired on every HP change (damage or heal). Delta is positive for healing, negative for damage. */
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	/** Fired once when HP reaches zero. */
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;

protected:
	virtual void BeginPlay() override;

private:
	float CurrentHealth = 0.0f;
	bool bDead = false;

	/** Bound to the owner's OnTakeAnyDamage delegate. */
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		AController* InstigatedBy, AActor* DamageCauser);
};
