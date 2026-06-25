// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RANDHUD.generated.h"

class URANDHUDWidget;
class URANDPhoneWidget;
class UInputMappingContext;
class UInputAction;

/**
 * ARANDHUD — creates the UMG HUD widget on BeginPlay, adds it to the viewport,
 * and binds it to the local player's ARANDCharacter so the heat bars, health
 * bar, and interaction prompt track the player's components.
 *
 * Also owns the phone widget (URANDPhoneWidget): created hidden, toggled with
 * Tab and closed with Tab/Escape.
 *
 * The widget classes default to the C++ widgets but can be overridden with
 * designer WBP subclasses.
 */
UCLASS()
class RAND_API ARANDHUD : public AHUD
{
	GENERATED_BODY()

public:
	ARANDHUD();

	/** The owned phone widget (may be null before BeginPlay). */
	URANDPhoneWidget* GetPhoneWidget() const { return PhoneWidget; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Widget class to instantiate. Defaults to URANDHUDWidget. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<URANDHUDWidget> HUDWidgetClass;

	/** Phone widget class. Defaults to URANDPhoneWidget. */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<URANDPhoneWidget> PhoneWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<URANDHUDWidget> HUDWidget;

	UPROPERTY()
	TObjectPtr<URANDPhoneWidget> PhoneWidget;

	// Phone toggle input (Tab open/close, Escape close).
	UPROPERTY()
	TObjectPtr<UInputMappingContext> PhoneMappingContext;

	UPROPERTY()
	TObjectPtr<UInputAction> TogglePhoneAction;

	UPROPERTY()
	TObjectPtr<UInputAction> ClosePhoneAction;

	FTimerHandle InputSetupTimer;

	/** Binds Tab/Escape on the local player (deferred until input exists). */
	void SetupPhoneInput();

	void TogglePhone();
	void ClosePhone();
};
