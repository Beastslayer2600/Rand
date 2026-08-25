// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDCheatComponent.h"
#include "RANDCharacter.h"
#include "EconomyComponent.h"
#include "WantedComponent.h"
#include "RANDCombatComponent.h"
#include "RANDInventoryComponent.h"
#include "RANDMissionManager.h"
#include "RANDPhoneWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

URANDCheatComponent::URANDCheatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDCheatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(InputSetupTimer, this, &URANDCheatComponent::SetupInput, 0.6f, false);
	}
}

void URANDCheatComponent::SetupInput()
{
	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC || !PC->GetLocalPlayer()) return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		CheatMappingContext = NewObject<UInputMappingContext>(this, TEXT("CheatMappingContext"));
		auto MakeBool = [this](const TCHAR* Name)
		{
			UInputAction* Action = NewObject<UInputAction>(this, Name);
			Action->ValueType = EInputActionValueType::Boolean;
			return Action;
		};

		CashAction = MakeBool(TEXT("IA_GiveCash"));
		ArmAction = MakeBool(TEXT("IA_Arm"));
		SkipAction = MakeBool(TEXT("IA_SkipObj"));
		HeatAction = MakeBool(TEXT("IA_ClearHeat"));
		Choice1Action = MakeBool(TEXT("IA_Phone1"));
		Choice2Action = MakeBool(TEXT("IA_Phone2"));
		ReloadAction = MakeBool(TEXT("IA_Reload"));

		CheatMappingContext->MapKey(CashAction, EKeys::F8);
		CheatMappingContext->MapKey(ArmAction, EKeys::F10);
		CheatMappingContext->MapKey(SkipAction, EKeys::F11);
		CheatMappingContext->MapKey(HeatAction, EKeys::F12);
		CheatMappingContext->MapKey(Choice1Action, EKeys::One);
		CheatMappingContext->MapKey(Choice2Action, EKeys::Two);
		CheatMappingContext->MapKey(ReloadAction, EKeys::R);

		Subsystem->AddMappingContext(CheatMappingContext, 4);

		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
		{
			EIC->BindAction(CashAction, ETriggerEvent::Started, this, &URANDCheatComponent::GiveCash);
			EIC->BindAction(ArmAction, ETriggerEvent::Started, this, &URANDCheatComponent::ArmPlayer);
			EIC->BindAction(SkipAction, ETriggerEvent::Started, this, &URANDCheatComponent::SkipObjective);
			EIC->BindAction(HeatAction, ETriggerEvent::Started, this, &URANDCheatComponent::ClearHeat);
			EIC->BindAction(Choice1Action, ETriggerEvent::Started, this, &URANDCheatComponent::PhoneChoice1);
			EIC->BindAction(Choice2Action, ETriggerEvent::Started, this, &URANDCheatComponent::PhoneChoice2);
			EIC->BindAction(ReloadAction, ETriggerEvent::Started, this, &URANDCheatComponent::ReloadWeapon);
		}
	}
}

void URANDCheatComponent::GiveCash()
{
	if (ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (URANDEconomyComponent* Econ = Player->GetEconomyComponent()) Econ->AddFunds(50000.0f, TEXT("Playtest float"));
	}
}

void URANDCheatComponent::ArmPlayer()
{
	if (ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (URANDCombatComponent* Combat = Player->GetCombatComponent())
		{
			Combat->SetArmed(true);
			Combat->AddReserve(24);
		}
		if (URANDInventoryComponent* Inv = Player->GetInventoryComponent()) Inv->AddItem(ERANDItem::Sidearm);
	}
}

void URANDCheatComponent::ReloadWeapon()
{
	if (ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (URANDCombatComponent* Combat = Player->GetCombatComponent()) Combat->Reload();
	}
}

void URANDCheatComponent::SkipObjective()
{
	if (URANDMissionManager* Missions = URANDMissionManager::Get(this))
	{
		const FName Active = Missions->GetActiveMissionID();
		if (Active != NAME_None) Missions->CompleteObjective(Active, 0);
	}
}

void URANDCheatComponent::ClearHeat()
{
	if (ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UWantedComponent* Wanted = Player->GetWantedComponent()) Wanted->ClearAllHeat();
	}
}

void URANDCheatComponent::PhoneChoice1() { PickPhoneOption(0); }
void URANDCheatComponent::PhoneChoice2() { PickPhoneOption(1); }

void URANDCheatComponent::PickPhoneOption(int32 Index)
{
	if (URANDPhoneWidget* Phone = URANDPhoneWidget::GetPhone(this)) Phone->ChooseOptionByIndex(Index);
}
