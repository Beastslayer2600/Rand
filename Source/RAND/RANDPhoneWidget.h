// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RANDPhoneWidget.generated.h"

class URANDHUDWidget;
class UScrollBox;
class UVerticalBox;

USTRUCT(BlueprintType)
struct FRANDMessageOption
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Phone")
	FText Label;

	UPROPERTY(BlueprintReadWrite, Category = "Phone")
	FName ActionId;
};

USTRUCT(BlueprintType)
struct FRANDMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Phone")
	FString Sender;

	UPROPERTY(BlueprintReadOnly, Category = "Phone")
	FText MessageText;

	UPROPERTY(BlueprintReadOnly, Category = "Phone")
	FString GameTimestamp;

	UPROPERTY(BlueprintReadOnly, Category = "Phone")
	bool bIsRead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Phone")
	TArray<FRANDMessageOption> Options;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhoneMessageOption, FName, ActionId);

UCLASS()
class RAND_API URANDPhoneWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	URANDPhoneWidget(const FObjectInitializer& ObjectInitializer);

	static URANDPhoneWidget* GetPhone(const UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "Phone")
	void ReceiveMessage(const FString& Sender, const FText& Text);

	UFUNCTION(BlueprintCallable, Category = "Phone")
	void ReceiveMessageWithOptions(const FString& Sender, const FText& Text,
		const TArray<FRANDMessageOption>& Options);

	UFUNCTION(BlueprintPure, Category = "Phone")
	int32 GetUnreadCount() const;

	UFUNCTION(BlueprintCallable, Category = "Phone")
	void OpenPhone();

	UFUNCTION(BlueprintCallable, Category = "Phone")
	void ClosePhone();

	UFUNCTION(BlueprintCallable, Category = "Phone")
	void TogglePhone();

	UFUNCTION(BlueprintPure, Category = "Phone")
	bool IsOpen() const { return bIsOpen; }

	/** Keyboard path for reply buttons (1 / 2). */
	UFUNCTION(BlueprintCallable, Category = "Phone")
	void ChooseOptionByIndex(int32 OptionIndex);

	void SetHUDWidget(URANDHUDWidget* InHUDWidget);

	UPROPERTY(BlueprintAssignable, Category = "Phone")
	FOnPhoneMessageOption OnMessageOptionSelected;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY()
	TObjectPtr<UScrollBox> MessageList;

	UPROPERTY()
	TObjectPtr<UVerticalBox> OptionBox;

private:
	UPROPERTY()
	TArray<FRANDMessage> Messages;

	bool bIsOpen = false;
	int32 PendingOptionMessage = INDEX_NONE;
	TWeakObjectPtr<URANDHUDWidget> HUDWidget;

	void AddMessage(const FString& Sender, const FText& Text, const TArray<FRANDMessageOption>& Options);
	void RebuildMessageList();
	void RebuildOptions();
	void MarkAllRead();
	FString CurrentTimestamp() const;

	UFUNCTION() void HandleOption0();
	UFUNCTION() void HandleOption1();
	UFUNCTION() void HandleOption2();
	UFUNCTION() void HandleOption3();
	void SelectOption(int32 OptionIndex);
};
