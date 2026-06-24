// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDHUD.h"
#include "RANDHUDWidget.h"
#include "RANDCharacter.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

ARANDHUD::ARANDHUD()
{
	// Default to the code-built widget; designers may override with a WBP.
	HUDWidgetClass = URANDHUDWidget::StaticClass();
}

void ARANDHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !HUDWidgetClass)
	{
		return;
	}

	HUDWidget = CreateWidget<URANDHUDWidget>(PC, HUDWidgetClass);
	if (!HUDWidget)
	{
		return;
	}

	HUDWidget->AddToViewport();

	if (ARANDCharacter* Character = Cast<ARANDCharacter>(PC->GetPawn()))
	{
		HUDWidget->BindToCharacter(Character);
	}
}

void ARANDHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HUDWidget)
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}
