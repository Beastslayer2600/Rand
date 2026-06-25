// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDHUDWidget.h"
#include "RANDCharacter.h"
#include "HealthComponent.h"
#include "InteractionComponent.h"
#include "TimeComponent.h"
#include "RANDMissionManager.h"
#include "RANDMinimapWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

namespace
{
	// Shared look for HUD labels.
	const FLinearColor LabelColor(0.92f, 0.92f, 0.92f);
}

URANDHUDWidget::URANDHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FText URANDHUDWidget::HeatLevelToText(EHeatLevel Level)
{
	switch (Level)
	{
	case EHeatLevel::Noticed: return NSLOCTEXT("RANDHUD", "Heat_Noticed", "Noticed");
	case EHeatLevel::Wanted:  return NSLOCTEXT("RANDHUD", "Heat_Wanted",  "Wanted");
	case EHeatLevel::Manhunt: return NSLOCTEXT("RANDHUD", "Heat_Manhunt", "Manhunt");
	case EHeatLevel::Burned:  return NSLOCTEXT("RANDHUD", "Heat_Burned",  "Burned");
	case EHeatLevel::None:
	default:                  return NSLOCTEXT("RANDHUD", "Heat_None",    "None");
	}
}

UHorizontalBox* URANDHUDWidget::BuildHeatRow(int32 AgencyIdx, const TCHAR* DisplayName, const FLinearColor& Color)
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), *FString::Printf(TEXT("HeatRow_%s"), DisplayName));

	// Agency name (fixed width so the bars line up).
	UTextBlock* NameLabel = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), *FString::Printf(TEXT("HeatName_%s"), DisplayName));
	NameLabel->SetText(FText::FromString(DisplayName));
	NameLabel->SetColorAndOpacity(FSlateColor(LabelColor));
	NameLabel->SetMinDesiredWidth(64.0f);
	if (UHorizontalBoxSlot* NameSlot = Row->AddChildToHorizontalBox(NameLabel))
	{
		NameSlot->SetVerticalAlignment(VAlign_Center);
		NameSlot->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));
	}

	// The bar — stretches to fill the row.
	UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(
		UProgressBar::StaticClass(), *FString::Printf(TEXT("HeatBar_%s"), DisplayName));
	Bar->SetFillColorAndOpacity(Color);
	Bar->SetPercent(0.0f);
	HeatBars[AgencyIdx] = Bar;
	if (UHorizontalBoxSlot* BarSlot = Row->AddChildToHorizontalBox(Bar))
	{
		BarSlot->SetVerticalAlignment(VAlign_Center);
		BarSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	// Level name (fixed width, right of the bar).
	UTextBlock* LevelText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), *FString::Printf(TEXT("HeatLevel_%s"), DisplayName));
	LevelText->SetText(HeatLevelToText(EHeatLevel::None));
	LevelText->SetColorAndOpacity(FSlateColor(LabelColor));
	LevelText->SetMinDesiredWidth(72.0f);
	HeatLevelTexts[AgencyIdx] = LevelText;
	if (UHorizontalBoxSlot* LevelSlot = Row->AddChildToHorizontalBox(LevelText))
	{
		LevelSlot->SetVerticalAlignment(VAlign_Center);
		LevelSlot->SetPadding(FMargin(6.0f, 0.0f, 0.0f, 0.0f));
	}

	return Row;
}

TSharedRef<SWidget> URANDHUDWidget::RebuildWidget()
{
	// A WidgetTree with a root panel is required for a code-built UUserWidget.
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(
			UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = Root;

		// --- Heat indicator (top-left): three agency rows ------------------
		const FLinearColor AgencyColors[AgencyCount] = {
			FLinearColor(0.10f, 0.35f, 0.90f), // SAPS  — blue
			FLinearColor(0.85f, 0.65f, 0.10f), // Hawks — gold
			FLinearColor(0.80f, 0.15f, 0.15f), // Rivals— red
		};
		const TCHAR* AgencyNames[AgencyCount] = { TEXT("SAPS"), TEXT("Hawks"), TEXT("Rivals") };

		HeatBars.SetNum(AgencyCount);
		HeatLevelTexts.SetNum(AgencyCount);

		UVerticalBox* HeatBox = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("HeatBox"));
		for (int32 i = 0; i < AgencyCount; ++i)
		{
			UHorizontalBox* Row = BuildHeatRow(i, AgencyNames[i], AgencyColors[i]);
			if (UVerticalBoxSlot* RowSlot = HeatBox->AddChildToVerticalBox(Row))
			{
				RowSlot->SetPadding(FMargin(0.0f, 3.0f));
			}
		}

		UCanvasPanelSlot* HeatCanvasSlot = Root->AddChildToCanvas(HeatBox);
		HeatCanvasSlot->SetPosition(FVector2D(40.0f, 40.0f));
		HeatCanvasSlot->SetSize(FVector2D(280.0f, 96.0f));

		// --- Health bar (bottom-left) with numeric overlay -----------------
		UOverlay* HealthOverlay = WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(), TEXT("HealthOverlay"));

		HealthBar = WidgetTree->ConstructWidget<UProgressBar>(
			UProgressBar::StaticClass(), TEXT("HealthBar"));
		HealthBar->SetFillColorAndOpacity(FLinearColor(0.15f, 0.75f, 0.20f));
		HealthBar->SetPercent(1.0f);
		if (UOverlaySlot* BarSlot = HealthOverlay->AddChildToOverlay(HealthBar))
		{
			BarSlot->SetHorizontalAlignment(HAlign_Fill);
			BarSlot->SetVerticalAlignment(VAlign_Fill);
		}

		HealthText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("HealthText"));
		HealthText->SetText(FText::GetEmpty());
		HealthText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		HealthText->SetJustification(ETextJustify::Center);
		if (UOverlaySlot* TextSlot = HealthOverlay->AddChildToOverlay(HealthText))
		{
			TextSlot->SetHorizontalAlignment(HAlign_Center);
			TextSlot->SetVerticalAlignment(VAlign_Center);
		}

		UCanvasPanelSlot* HealthCanvasSlot = Root->AddChildToCanvas(HealthOverlay);
		HealthCanvasSlot->SetAnchors(FAnchors(0.0f, 1.0f));
		HealthCanvasSlot->SetAlignment(FVector2D(0.0f, 1.0f));
		HealthCanvasSlot->SetPosition(FVector2D(40.0f, -40.0f));
		HealthCanvasSlot->SetSize(FVector2D(300.0f, 26.0f));

		// --- Interaction prompt (bottom-center) ----------------------------
		InteractionText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("InteractionText"));
		InteractionText->SetText(FText::GetEmpty());
		InteractionText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		InteractionText->SetJustification(ETextJustify::Center);
		InteractionText->SetVisibility(ESlateVisibility::Collapsed);

		UCanvasPanelSlot* PromptCanvasSlot = Root->AddChildToCanvas(InteractionText);
		PromptCanvasSlot->SetAnchors(FAnchors(0.5f, 1.0f));
		PromptCanvasSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		PromptCanvasSlot->SetPosition(FVector2D(0.0f, -120.0f));
		PromptCanvasSlot->SetAutoSize(true);

		// --- Minimap (top-right, 200x200) ----------------------------------
		Minimap = WidgetTree->ConstructWidget<URANDMinimapWidget>(
			URANDMinimapWidget::StaticClass(), TEXT("Minimap"));

		UCanvasPanelSlot* MinimapSlot = Root->AddChildToCanvas(Minimap);
		MinimapSlot->SetAnchors(FAnchors(1.0f, 0.0f));
		MinimapSlot->SetAlignment(FVector2D(1.0f, 0.0f));
		MinimapSlot->SetPosition(FVector2D(-40.0f, 40.0f));
		MinimapSlot->SetSize(FVector2D(200.0f, 200.0f));

		// --- Clock (top-right, just below the minimap): "Day 1 - 14:32" -----
		TimeText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("TimeText"));
		TimeText->SetText(FText::GetEmpty());
		TimeText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		TimeText->SetJustification(ETextJustify::Right);

		UCanvasPanelSlot* TimeCanvasSlot = Root->AddChildToCanvas(TimeText);
		TimeCanvasSlot->SetAnchors(FAnchors(1.0f, 0.0f));
		TimeCanvasSlot->SetAlignment(FVector2D(1.0f, 0.0f));
		TimeCanvasSlot->SetPosition(FVector2D(-40.0f, 252.0f));
		TimeCanvasSlot->SetAutoSize(true);

		// --- Active mission (bottom-right) ---------------------------------
		MissionBox = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("MissionBox"));
		MissionBox->SetVisibility(ESlateVisibility::Collapsed);

		UCanvasPanelSlot* MissionCanvasSlot = Root->AddChildToCanvas(MissionBox);
		MissionCanvasSlot->SetAnchors(FAnchors(1.0f, 1.0f));
		MissionCanvasSlot->SetAlignment(FVector2D(1.0f, 1.0f));
		MissionCanvasSlot->SetPosition(FVector2D(-40.0f, -40.0f));
		MissionCanvasSlot->SetAutoSize(true);

		// --- Phone notification (top-center) -------------------------------
		PhoneNotificationText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("PhoneNotificationText"));
		PhoneNotificationText->SetText(
			NSLOCTEXT("RANDHUD", "NewMessage", "● New message  (Tab)"));
		PhoneNotificationText->SetColorAndOpacity(FSlateColor(FLinearColor(0.30f, 0.85f, 0.45f)));
		PhoneNotificationText->SetJustification(ETextJustify::Center);
		PhoneNotificationText->SetVisibility(ESlateVisibility::Collapsed);

		UCanvasPanelSlot* NotifSlot = Root->AddChildToCanvas(PhoneNotificationText);
		NotifSlot->SetAnchors(FAnchors(0.5f, 0.0f));
		NotifSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		NotifSlot->SetPosition(FVector2D(0.0f, 20.0f));
		NotifSlot->SetAutoSize(true);
	}

	return Super::RebuildWidget();
}

void URANDHUDWidget::BindToCharacter(ARANDCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	BoundHealth = Character->GetHealthComponent();
	BoundInteraction = Character->GetInteractionComponent();
	BoundWanted = Character->GetWantedComponent();

	if (UHealthComponent* Health = BoundHealth.Get())
	{
		Health->OnHealthChanged.AddDynamic(this, &URANDHUDWidget::HandleHealthChanged);
	}
	if (UInteractionComponent* Interaction = BoundInteraction.Get())
	{
		Interaction->OnTargetChanged.AddDynamic(this, &URANDHUDWidget::HandleTargetChanged);
	}
	if (UWantedComponent* Wanted = BoundWanted.Get())
	{
		Wanted->OnHeatChanged.AddDynamic(this, &URANDHUDWidget::HandleHeatChanged);
	}

	// The clock and missions live on the game mode, not the character.
	BoundTime = URANDTimeComponent::Get(this);
	if (URANDTimeComponent* Clock = BoundTime.Get())
	{
		Clock->OnMinutePassed.AddDynamic(this, &URANDHUDWidget::HandleMinutePassed);
	}

	BoundMissions = URANDMissionManager::Get(this);
	if (URANDMissionManager* Missions = BoundMissions.Get())
	{
		Missions->OnMissionStarted.AddDynamic(this, &URANDHUDWidget::HandleMissionStarted);
		Missions->OnObjectiveComplete.AddDynamic(this, &URANDHUDWidget::HandleObjectiveComplete);
		Missions->OnMissionComplete.AddDynamic(this, &URANDHUDWidget::HandleMissionComplete);
		Missions->OnMissionFailed.AddDynamic(this, &URANDHUDWidget::HandleMissionFailed);
	}

	RefreshAll();
}

void URANDHUDWidget::NativeDestruct()
{
	if (UHealthComponent* Health = BoundHealth.Get())
	{
		Health->OnHealthChanged.RemoveDynamic(this, &URANDHUDWidget::HandleHealthChanged);
	}
	if (UInteractionComponent* Interaction = BoundInteraction.Get())
	{
		Interaction->OnTargetChanged.RemoveDynamic(this, &URANDHUDWidget::HandleTargetChanged);
	}
	if (UWantedComponent* Wanted = BoundWanted.Get())
	{
		Wanted->OnHeatChanged.RemoveDynamic(this, &URANDHUDWidget::HandleHeatChanged);
	}
	if (URANDTimeComponent* Clock = BoundTime.Get())
	{
		Clock->OnMinutePassed.RemoveDynamic(this, &URANDHUDWidget::HandleMinutePassed);
	}
	if (URANDMissionManager* Missions = BoundMissions.Get())
	{
		Missions->OnMissionStarted.RemoveDynamic(this, &URANDHUDWidget::HandleMissionStarted);
		Missions->OnObjectiveComplete.RemoveDynamic(this, &URANDHUDWidget::HandleObjectiveComplete);
		Missions->OnMissionComplete.RemoveDynamic(this, &URANDHUDWidget::HandleMissionComplete);
		Missions->OnMissionFailed.RemoveDynamic(this, &URANDHUDWidget::HandleMissionFailed);
	}

	Super::NativeDestruct();
}

void URANDHUDWidget::RefreshAll()
{
	if (UHealthComponent* Health = BoundHealth.Get())
	{
		HandleHealthChanged(Health->GetHealth(), 0.0f);
	}

	if (UWantedComponent* Wanted = BoundWanted.Get())
	{
		for (int32 i = 0; i < AgencyCount; ++i)
		{
			HandleHeatChanged(static_cast<EAgency>(i), EHeatLevel::None,
				Wanted->GetHeatLevel(static_cast<EAgency>(i)));
		}
	}

	if (UInteractionComponent* Interaction = BoundInteraction.Get())
	{
		HandleTargetChanged(Interaction->GetCurrentTarget());
	}

	if (URANDTimeComponent* Clock = BoundTime.Get())
	{
		HandleMinutePassed(Clock->GetCurrentDay(), Clock->GetCurrentHour(), Clock->GetCurrentMinute());
	}

	UpdateMissionDisplay();
}

// --- Delegate handlers ------------------------------------------------------

void URANDHUDWidget::HandleHealthChanged(float /*NewHealth*/, float /*Delta*/)
{
	UHealthComponent* Health = BoundHealth.Get();
	if (!Health)
	{
		return;
	}

	if (HealthBar)
	{
		HealthBar->SetPercent(Health->GetHealthPercent());
	}
	if (HealthText)
	{
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"),
			FMath::RoundToInt(Health->GetHealth()), FMath::RoundToInt(Health->GetMaxHealth()))));
	}
}

void URANDHUDWidget::HandleHeatChanged(EAgency Agency, EHeatLevel /*OldLevel*/, EHeatLevel NewLevel)
{
	const int32 Idx = static_cast<int32>(Agency);
	if (HeatBars.IsValidIndex(Idx) && HeatBars[Idx])
	{
		const float Fraction = static_cast<float>(NewLevel) / static_cast<float>(MaxHeatLevel);
		HeatBars[Idx]->SetPercent(Fraction);
	}
	if (HeatLevelTexts.IsValidIndex(Idx) && HeatLevelTexts[Idx])
	{
		HeatLevelTexts[Idx]->SetText(HeatLevelToText(NewLevel));
	}
}

void URANDHUDWidget::HandleTargetChanged(AActor* NewTarget)
{
	if (!InteractionText)
	{
		return;
	}

	if (NewTarget && BoundInteraction.IsValid())
	{
		InteractionText->SetText(BoundInteraction->GetPromptText());
		InteractionText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		InteractionText->SetText(FText::GetEmpty());
		InteractionText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void URANDHUDWidget::HandleMinutePassed(int32 /*Day*/, int32 /*Hour*/, int32 /*Minute*/)
{
	if (TimeText)
	{
		if (URANDTimeComponent* Clock = BoundTime.Get())
		{
			TimeText->SetText(FText::FromString(Clock->GetTimeString()));
		}
	}
}

void URANDHUDWidget::SetPhoneNotification(bool bHasUnread)
{
	if (PhoneNotificationText)
	{
		PhoneNotificationText->SetVisibility(
			bHasUnread ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

// --- Mission display --------------------------------------------------------

void URANDHUDWidget::HandleMissionStarted(FName /*MissionID*/)        { UpdateMissionDisplay(); }
void URANDHUDWidget::HandleObjectiveComplete(FName /*MissionID*/, int32 /*Index*/) { UpdateMissionDisplay(); }
void URANDHUDWidget::HandleMissionComplete(FName /*MissionID*/)       { UpdateMissionDisplay(); }
void URANDHUDWidget::HandleMissionFailed(FName /*MissionID*/)         { UpdateMissionDisplay(); }

void URANDHUDWidget::UpdateMissionDisplay()
{
	if (!MissionBox)
	{
		return;
	}

	MissionBox->ClearChildren();

	FRANDMission Mission;
	URANDMissionManager* Missions = BoundMissions.Get();
	if (!Missions || !Missions->GetActiveMission(Mission))
	{
		MissionBox->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	MissionBox->SetVisibility(ESlateVisibility::HitTestInvisible);

	// Title.
	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Title->SetText(Mission.MissionName);
	Title->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.30f)));
	Title->SetJustification(ETextJustify::Right);
	MissionBox->AddChildToVerticalBox(Title);

	// Objectives, each with a tick box and an "(optional)" suffix.
	for (const FRANDObjective& Objective : Mission.Objectives)
	{
		UTextBlock* Line = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		const FString Box = Objective.bIsComplete ? TEXT("[x] ") : TEXT("[ ] ");
		const FString Optional = Objective.bIsOptional ? TEXT("  (optional)") : TEXT("");
		Line->SetText(FText::FromString(Box + Objective.ObjectiveText.ToString() + Optional));
		Line->SetColorAndOpacity(FSlateColor(
			Objective.bIsComplete ? FLinearColor(0.55f, 0.85f, 0.55f) : LabelColor));
		Line->SetJustification(ETextJustify::Right);
		MissionBox->AddChildToVerticalBox(Line);
	}
}
