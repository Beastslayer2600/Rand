// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDSaveGameManager.h"
#include "RANDSaveGame.h"
#include "RANDCharacter.h"
#include "EconomyComponent.h"
#include "BusinessManager.h"
#include "WantedComponent.h"
#include "TimeComponent.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

const FString URANDSaveGame::SlotName = TEXT("RANDSave_Slot0");

URANDSaveGameManager::URANDSaveGameManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDSaveGameManager::BeginPlay()
{
	Super::BeginPlay();

	// Auto-save every in-game hour.
	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		Clock->OnHourPassed.AddDynamic(this, &URANDSaveGameManager::HandleHourPassed);
	}

	// The player controller's input component may not exist yet at GameMode
	// BeginPlay; defer the F5 binding briefly so it's reliably available.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(InputSetupTimer, this,
			&URANDSaveGameManager::SetupInput, 0.5f, /*bLoop=*/false);
	}
}

void URANDSaveGameManager::SetupInput()
{
	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC || !PC->GetLocalPlayer())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		SaveMappingContext = NewObject<UInputMappingContext>(this, TEXT("SaveMappingContext"));
		SaveAction = NewObject<UInputAction>(this, TEXT("IA_QuickSave"));
		SaveAction->ValueType = EInputActionValueType::Boolean;
		SaveMappingContext->MapKey(SaveAction, EKeys::F5);

		Subsystem->AddMappingContext(SaveMappingContext, 2);

		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
		{
			EIC->BindAction(SaveAction, ETriggerEvent::Started, this,
				&URANDSaveGameManager::HandleManualSave);
		}
	}
}

void URANDSaveGameManager::HandleManualSave()
{
	SaveGame();
}

void URANDSaveGameManager::HandleHourPassed(int32 /*Day*/, int32 /*Hour*/)
{
	SaveGame();
}

void URANDSaveGameManager::SaveGame()
{
	URANDSaveGame* Save =
		Cast<URANDSaveGame>(UGameplayStatics::CreateSaveGameObject(URANDSaveGame::StaticClass()));
	if (!Save)
	{
		return;
	}

	if (ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (URANDEconomyComponent* Econ = Player->GetEconomyComponent())
		{
			Save->Balance = Econ->GetBalance();

			// Keep only the most recent 50 ledger entries.
			const TArray<FRANDTransaction>& Log = Econ->GetTransactionLog();
			const int32 Start = FMath::Max(0, Log.Num() - 50);
			for (int32 i = Start; i < Log.Num(); ++i)
			{
				Save->TransactionLog.Add(Log[i]);
			}
		}
		if (URANDBusinessManager* Biz = Player->GetBusinessManager())
		{
			Save->OwnedBusinesses = Biz->GetBusinesses();
		}
		if (UWantedComponent* Wanted = Player->GetWantedComponent())
		{
			Save->HeatSAPS = Wanted->GetRawHeat(EAgency::SAPS);
			Save->HeatHawks = Wanted->GetRawHeat(EAgency::Hawks);
			Save->HeatRivals = Wanted->GetRawHeat(EAgency::Rivals);
		}
		Save->PlayerLocation = Player->GetActorLocation();
	}

	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		Save->Day = Clock->GetCurrentDay();
		Save->Hour = Clock->GetCurrentHour();
		Save->Minute = Clock->GetCurrentMinute();
	}

	UGameplayStatics::SaveGameToSlot(Save, URANDSaveGame::SlotName, 0);
}

void URANDSaveGameManager::LoadGame()
{
	if (!UGameplayStatics::DoesSaveGameExist(URANDSaveGame::SlotName, 0))
	{
		return;
	}

	URANDSaveGame* Save =
		Cast<URANDSaveGame>(UGameplayStatics::LoadGameFromSlot(URANDSaveGame::SlotName, 0));
	if (!Save)
	{
		return;
	}

	if (ARANDCharacter* Player = Cast<ARANDCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (URANDEconomyComponent* Econ = Player->GetEconomyComponent())
		{
			Econ->LoadState(Save->Balance, Save->TransactionLog);
		}
		if (URANDBusinessManager* Biz = Player->GetBusinessManager())
		{
			Biz->SetBusinesses(Save->OwnedBusinesses);
		}
		if (UWantedComponent* Wanted = Player->GetWantedComponent())
		{
			Wanted->SetHeat(EAgency::SAPS, Save->HeatSAPS);
			Wanted->SetHeat(EAgency::Hawks, Save->HeatHawks);
			Wanted->SetHeat(EAgency::Rivals, Save->HeatRivals);
		}
		Player->SetActorLocation(Save->PlayerLocation);
	}

	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		Clock->SetTime(Save->Day, Save->Hour, Save->Minute);
	}
}
