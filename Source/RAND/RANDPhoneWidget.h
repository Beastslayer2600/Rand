// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RANDPhoneWidget.generated.h"

class URANDHUDWidget;
class UScrollBox;
class UVerticalBox;

/** A tappable reply on a phone message (e.g. "Accept" / "Decline"). */
USTRUCT(BlueprintType)
struct FRANDMessageOption
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Phone")
	FText Label;

	/** Identifier broadcast via OnMessageOptionSelected when tapped. */
	UPROPERTY(BlueprintReadWrite, Category = "Phone")
	FName ActionId;
};

/** One entry in the WhatsApp-style message thread. */
USTRUCT(BlueprintType)
struct FRANDMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Phone")
	FString Sender;

	UPROPERTY(BlueprintReadOnly, Category = "Phone")
	FText MessageText;

	/** In-game time the message arrived, e.g. "Day 1 — 14:32". */
	UPROPERTY(BlueprintReadOnly, Category = "Phone")
	FString GameTimestamp;

	UPROPERTY(BlueprintReadOnly, Category = "Phone")
	bool bIsRead = false;

	/** Optional reply buttons (e.g. a bribe accept/decline prompt). */
	UPROPERTY(BlueprintReadOnly, Category = "Phone")
	TArray<FRANDMessageOption> Options;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhoneMessageOption, FName, ActionId);

/**
 * URANDPhoneWidget — André's phone, a WhatsApp-style message thread built as a
 * code-only UMG widget. Owned by ARANDHUD (created hidden, toggled with Tab,
 * closed with Tab/Escape).
 *
 * Gameplay delivers messages via ReceiveMessage / ReceiveMessageWithOptions;
 * unread messages raise a HUD notification. Messages may carry reply options
 * (rendered as buttons) whose taps broadcast OnMessageOptionSelected — used to
 * deliver Thandi's bribe offer and route the player's choice back to the mission.
 */
UCLASS()
class RAND_API URANDPhoneWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	URANDPhoneWidget(const FObjectInitializer& ObjectInitializer);

	/** Resolves the local player's phone via the active ARANDHUD. */
	static URANDPhoneWidget* GetPhone(const UObject* WorldContext);

	// --- Messaging ----------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Phone")
	void ReceiveMessage(const FString& Sender, const FText& Text);

	UFUNCTION(BlueprintCallable, Category = "Phone")
	void ReceiveMessageWithOptions(const FString& Sender, const FText& Text,
		const TArray<FRANDMessageOption>& Options);

	UFUNCTION(BlueprintPure, Category = "Phone")
	int32 GetUnreadCount() const;

	// --- Visibility ---------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Phone")
	void OpenPhone();

	UFUNCTION(BlueprintCallable, Category = "Phone")
	void ClosePhone();

	UFUNCTION(BlueprintCallable, Category = "Phone")
	void TogglePhone();

	UFUNCTION(BlueprintPure, Category = "Phone")
	bool IsOpen() const { return bIsOpen; }

	/** Wired by ARANDHUD so the phone can raise the unread notification. */
	void SetHUDWidget(URANDHUDWidget* InHUDWidget);

	// --- Delegates ----------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Phone")
	FOnPhoneMessageOption OnMessageOptionSelected;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY()
	TObjectPtr<UScrollBox> MessageList;

	/** Holds the reply buttons for the message currently awaiting a response. */
	UPROPERTY()
	TObjectPtr<UVerticalBox> OptionBox;

private:
	UPROPERTY()
	TArray<FRANDMessage> Messages;

	bool bIsOpen = false;

	/** Index into Messages of the message whose options are awaiting a tap. */
	int32 PendingOptionMessage = INDEX_NONE;

	TWeakObjectPtr<URANDHUDWidget> HUDWidget;

	void AddMessage(const FString& Sender, const FText& Text, const TArray<FRANDMessageOption>& Options);
	void RebuildMessageList();
	void RebuildOptions();
	void MarkAllRead();
	FString CurrentTimestamp() const;

	// Fixed option-button handlers (the active prompt has at most a few replies).
	UFUNCTION() void HandleOption0();
	UFUNCTION() void HandleOption1();
	UFUNCTION() void HandleOption2();
	UFUNCTION() void HandleOption3();
	void SelectOption(int32 OptionIndex);
};
