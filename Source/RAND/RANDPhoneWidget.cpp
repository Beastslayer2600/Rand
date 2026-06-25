// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDPhoneWidget.h"
#include "RANDHUDWidget.h"
#include "RANDHUD.h"
#include "TimeComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

URANDPhoneWidget::URANDPhoneWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

URANDPhoneWidget* URANDPhoneWidget::GetPhone(const UObject* WorldContext)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContext, 0))
	{
		if (ARANDHUD* HUD = Cast<ARANDHUD>(PC->GetHUD()))
		{
			return HUD->GetPhoneWidget();
		}
	}
	return nullptr;
}

TSharedRef<SWidget> URANDPhoneWidget::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(
			UCanvasPanel::StaticClass(), TEXT("PhoneRoot"));
		WidgetTree->RootWidget = Root;

		// Phone frame: a dark panel on the right third of the screen.
		UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PhoneFrame"));
		Frame->SetBrushColor(FLinearColor(0.05f, 0.07f, 0.06f, 0.96f));
		Frame->SetPadding(FMargin(12.0f));

		UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("PhoneColumn"));
		Frame->SetContent(Column);

		// Header.
		UTextBlock* Header = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PhoneHeader"));
		Header->SetText(NSLOCTEXT("RANDPhone", "Messages", "Messages"));
		Header->SetColorAndOpacity(FSlateColor(FLinearColor(0.30f, 0.85f, 0.45f)));
		if (UVerticalBoxSlot* HSlot = Column->AddChildToVerticalBox(Header))
		{
			HSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		}

		// Scrollable message thread (fills the column).
		MessageList = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("MessageList"));
		if (UVerticalBoxSlot* LSlot = Column->AddChildToVerticalBox(MessageList))
		{
			LSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}

		// Reply buttons for the active prompt.
		OptionBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OptionBox"));
		if (UVerticalBoxSlot* OSlot = Column->AddChildToVerticalBox(OptionBox))
		{
			OSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
		}

		// Fixed-size panel hugging the top-right corner (unambiguous: a point
		// anchor + alignment, no stretch).
		UCanvasPanelSlot* FrameSlot = Root->AddChildToCanvas(Frame);
		FrameSlot->SetAnchors(FAnchors(1.0f, 0.0f));
		FrameSlot->SetAlignment(FVector2D(1.0f, 0.0f));
		FrameSlot->SetPosition(FVector2D(0.0f, 0.0f));
		FrameSlot->SetSize(FVector2D(380.0f, 720.0f));
	}

	// Start hidden; ARANDHUD/Tab opens it.
	SetVisibility(bIsOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	return Super::RebuildWidget();
}

// --- Messaging --------------------------------------------------------------

void URANDPhoneWidget::ReceiveMessage(const FString& Sender, const FText& Text)
{
	AddMessage(Sender, Text, {});
}

void URANDPhoneWidget::ReceiveMessageWithOptions(const FString& Sender, const FText& Text,
	const TArray<FRANDMessageOption>& Options)
{
	AddMessage(Sender, Text, Options);
}

void URANDPhoneWidget::AddMessage(const FString& Sender, const FText& Text,
	const TArray<FRANDMessageOption>& Options)
{
	FRANDMessage Message;
	Message.Sender = Sender;
	Message.MessageText = Text;
	Message.GameTimestamp = CurrentTimestamp();
	Message.bIsRead = bIsOpen; // read immediately if the player is looking at it
	Message.Options = Options;

	const int32 Index = Messages.Add(Message);
	if (Options.Num() > 0)
	{
		PendingOptionMessage = Index;
	}

	RebuildMessageList();
	RebuildOptions();

	// Raise the HUD notification for messages that arrive while the phone is shut.
	if (!bIsOpen && HUDWidget.IsValid())
	{
		HUDWidget->SetPhoneNotification(true);
	}
}

int32 URANDPhoneWidget::GetUnreadCount() const
{
	int32 Count = 0;
	for (const FRANDMessage& Message : Messages)
	{
		if (!Message.bIsRead)
		{
			++Count;
		}
	}
	return Count;
}

void URANDPhoneWidget::MarkAllRead()
{
	for (FRANDMessage& Message : Messages)
	{
		Message.bIsRead = true;
	}
}

FString URANDPhoneWidget::CurrentTimestamp() const
{
	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		return Clock->GetTimeString();
	}
	return FString();
}

// --- Visibility -------------------------------------------------------------

void URANDPhoneWidget::OpenPhone()
{
	bIsOpen = true;
	SetVisibility(ESlateVisibility::Visible);
	MarkAllRead();
	RebuildMessageList();
	RebuildOptions();
	if (HUDWidget.IsValid())
	{
		HUDWidget->SetPhoneNotification(false);
	}
}

void URANDPhoneWidget::ClosePhone()
{
	bIsOpen = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

void URANDPhoneWidget::TogglePhone()
{
	if (bIsOpen)
	{
		ClosePhone();
	}
	else
	{
		OpenPhone();
	}
}

void URANDPhoneWidget::SetHUDWidget(URANDHUDWidget* InHUDWidget)
{
	HUDWidget = InHUDWidget;
}

// --- Tree rebuilds ----------------------------------------------------------

void URANDPhoneWidget::RebuildMessageList()
{
	if (!MessageList)
	{
		return;
	}
	MessageList->ClearChildren();

	for (const FRANDMessage& Message : Messages)
	{
		UVerticalBox* Bubble = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());

		UTextBlock* SenderLine = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		SenderLine->SetText(FText::FromString(Message.Sender));
		SenderLine->SetColorAndOpacity(FSlateColor(FLinearColor(0.30f, 0.85f, 0.45f)));
		Bubble->AddChildToVerticalBox(SenderLine);

		UTextBlock* Body = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Body->SetText(Message.MessageText);
		Body->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.95f, 0.95f)));
		Body->SetAutoWrapText(true);
		Bubble->AddChildToVerticalBox(Body);

		UTextBlock* Stamp = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Stamp->SetText(FText::FromString(Message.GameTimestamp));
		Stamp->SetColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.55f, 0.55f)));
		Bubble->AddChildToVerticalBox(Stamp);

		if (UPanelSlot* BubbleSlot = MessageList->AddChild(Bubble))
		{
			if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(BubbleSlot))
			{
				ScrollSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
			}
		}
	}

	MessageList->ScrollToEnd();
}

void URANDPhoneWidget::RebuildOptions()
{
	if (!OptionBox)
	{
		return;
	}
	OptionBox->ClearChildren();

	if (!bIsOpen || !Messages.IsValidIndex(PendingOptionMessage))
	{
		return;
	}

	const TArray<FRANDMessageOption>& Options = Messages[PendingOptionMessage].Options;
	for (int32 i = 0; i < Options.Num() && i < 4; ++i)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());

		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Label->SetText(Options[i].Label);
		Button->AddChild(Label);

		switch (i)
		{
		case 0: Button->OnClicked.AddDynamic(this, &URANDPhoneWidget::HandleOption0); break;
		case 1: Button->OnClicked.AddDynamic(this, &URANDPhoneWidget::HandleOption1); break;
		case 2: Button->OnClicked.AddDynamic(this, &URANDPhoneWidget::HandleOption2); break;
		case 3: Button->OnClicked.AddDynamic(this, &URANDPhoneWidget::HandleOption3); break;
		default: break;
		}

		if (UVerticalBoxSlot* ButtonSlot = OptionBox->AddChildToVerticalBox(Button))
		{
			ButtonSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
		}
	}
}

void URANDPhoneWidget::HandleOption0() { SelectOption(0); }
void URANDPhoneWidget::HandleOption1() { SelectOption(1); }
void URANDPhoneWidget::HandleOption2() { SelectOption(2); }
void URANDPhoneWidget::HandleOption3() { SelectOption(3); }

void URANDPhoneWidget::SelectOption(int32 OptionIndex)
{
	if (!Messages.IsValidIndex(PendingOptionMessage))
	{
		return;
	}

	const TArray<FRANDMessageOption>& Options = Messages[PendingOptionMessage].Options;
	if (!Options.IsValidIndex(OptionIndex))
	{
		return;
	}

	const FName ActionId = Options[OptionIndex].ActionId;

	// Consume the prompt before broadcasting so re-entrancy can't double-fire.
	PendingOptionMessage = INDEX_NONE;
	RebuildOptions();

	OnMessageOptionSelected.Broadcast(ActionId);
}
