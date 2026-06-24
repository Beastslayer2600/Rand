// Copyright Fortitudo Studio. All Rights Reserved.

#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/DamageType.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
	}
}

float UHealthComponent::GetHealthPercent() const
{
	return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

void UHealthComponent::Heal(float Amount)
{
	if (bDead || Amount <= 0.0f)
	{
		return;
	}

	const float Previous = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
	const float Delta = CurrentHealth - Previous;

	if (Delta != 0.0f)
	{
		OnHealthChanged.Broadcast(CurrentHealth, Delta);
	}
}

void UHealthComponent::HandleTakeAnyDamage(AActor* /*DamagedActor*/, float Damage,
	const UDamageType* /*DamageType*/, AController* /*InstigatedBy*/, AActor* /*DamageCauser*/)
{
	if (bDead || Damage <= 0.0f)
	{
		return;
	}

	const float Previous = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
	const float Delta = CurrentHealth - Previous; // negative

	OnHealthChanged.Broadcast(CurrentHealth, Delta);

	if (CurrentHealth <= 0.0f)
	{
		bDead = true;
		OnDeath.Broadcast();
	}
}
