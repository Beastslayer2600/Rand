// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDHUD.h"
#include "RANDHUDWidget.h"
#include "RANDPhoneWidget.h"
#include "RANDCharacter.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

ARANDHUD::ARANDHUD()
{
	// Default to the code-built widgets; designers may override with WBPs.
	HUDWidgetClass = URANDHUDWidget::StaticClass();
	PhoneWidgetClass = URANDPhoneWidget::StaticClass();
}

void ARANDHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<URANDHUDWidget>(PC, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport(0);
			if (ARANDCharacter* Character = Cast<ARANDCharacter>(PC->GetPawn()))
			{
				HUDWidget->BindToCharacter(Character);
			}
		}
	}

	if (PhoneWidgetClass)
	{
		PhoneWidget = CreateWidget<URANDPhoneWidget>(PC, PhoneWidgetClass);
		if (PhoneWidget)
		{
			PhoneWidget->AddToViewport(10); // above the HUD
			PhoneWidget->SetHUDWidget(HUDWidget);
		}
	}

	// Bind Tab/Escape once the player's input component is ready.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(InputSetupTimer, this,
			&ARANDHUD::SetupPhoneInput, 0.5f, /*bLoop=*/false);
	}
}

void ARANDHUD::SetupPhoneInput()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->GetLocalPlayer())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		PhoneMappingContext = NewObject<UInputMappingContext>(this, TEXT("PhoneMappingContext"));

		TogglePhoneAction = NewObject<UInputAction>(this, TEXT("IA_TogglePhone"));
		TogglePhoneAction->ValueType = EInputActionValueType::Boolean;
		PhoneMappingContext->MapKey(TogglePhoneAction, EKeys::Tab);

		ClosePhoneAction = NewObject<UInputAction>(this, TEXT("IA_ClosePhone"));
		ClosePhoneAction->ValueType = EInputActionValueType::Boolean;
		PhoneMappingContext->MapKey(ClosePhoneAction, EKeys::Escape);

		Subsystem->AddMappingContext(PhoneMappingContext, 3);

		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
		{
			EIC->BindAction(TogglePhoneAction, ETriggerEvent::Started, this, &ARANDHUD::TogglePhone);
			EIC->BindAction(ClosePhoneAction, ETriggerEvent::Started, this, &ARANDHUD::ClosePhone);
		}
	}
}

void ARANDHUD::TogglePhone()
{
	if (PhoneWidget)
	{
		PhoneWidget->TogglePhone();
	}
}

void ARANDHUD::ClosePhone()
{
	if (PhoneWidget)
	{
		PhoneWidget->ClosePhone();
	}
}

void ARANDHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HUDWidget)
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}
	if (PhoneWidget)
	{
		PhoneWidget->RemoveFromParent();
		PhoneWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}
