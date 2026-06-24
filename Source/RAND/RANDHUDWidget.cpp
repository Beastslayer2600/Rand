// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDHUDWidget.h"
#include "RANDCharacter.h"
#include "HealthComponent.h"
#include "InteractionComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

URANDHUDWidget::URANDHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> URANDHUDWidget::RebuildWidget()
{
	// A WidgetTree with a root panel is required for a code-built UUserWidget.
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(
			UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = Root;

		// --- Heat indicator (top-left): three stacked agency bars ----------
		UVerticalBox* HeatBox = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("HeatBox"));

		const FLinearColor AgencyColors[AgencyCount] = {
			FLinearColor(0.10f, 0.35f, 0.90f), // SAPS  — blue
			FLinearColor(0.85f, 0.65f, 0.10f), // Hawks — gold
			FLinearColor(0.80f, 0.15f, 0.15f), // Rivals— red
		};
		const TCHAR* AgencyNames[AgencyCount] = { TEXT("SAPS"), TEXT("Hawks"), TEXT("Rivals") };

		HeatBars.SetNum(AgencyCount);
		for (int32 i = 0; i < AgencyCount; ++i)
		{
			UVerticalBoxSlot* RowSlot = HeatBox->AddChildToVerticalBox(
				[&]() -> UProgressBar*
				{
					UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(
						UProgressBar::StaticClass(), *FString::Printf(TEXT("HeatBar_%s"), AgencyNames[i]));
					Bar->SetFillColorAndOpacity(AgencyColors[i]);
					Bar->SetPercent(0.0f);
					HeatBars[i] = Bar;
					return Bar;
				}());
			if (RowSlot)
			{
				RowSlot->SetPadding(FMargin(0.0f, 2.0f));
			}
		}

		UCanvasPanelSlot* HeatCanvasSlot = Root->AddChildToCanvas(HeatBox);
		HeatCanvasSlot->SetPosition(FVector2D(40.0f, 40.0f));
		HeatCanvasSlot->SetSize(FVector2D(220.0f, 80.0f));

		// --- Health bar (bottom-left) --------------------------------------
		HealthBar = WidgetTree->ConstructWidget<UProgressBar>(
			UProgressBar::StaticClass(), TEXT("HealthBar"));
		HealthBar->SetFillColorAndOpacity(FLinearColor(0.15f, 0.75f, 0.20f));
		HealthBar->SetPercent(1.0f);

		UCanvasPanelSlot* HealthCanvasSlot = Root->AddChildToCanvas(HealthBar);
		HealthCanvasSlot->SetAnchors(FAnchors(0.0f, 1.0f));
		HealthCanvasSlot->SetAlignment(FVector2D(0.0f, 1.0f));
		HealthCanvasSlot->SetPosition(FVector2D(40.0f, -40.0f));
		HealthCanvasSlot->SetSize(FVector2D(300.0f, 24.0f));

		// --- Interaction prompt (bottom-center) ----------------------------
		InteractionText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("InteractionText"));
		InteractionText->SetText(FText::GetEmpty());
		InteractionText->SetJustification(ETextJustify::Center);
		InteractionText->SetVisibility(ESlateVisibility::Collapsed);

		UCanvasPanelSlot* PromptCanvasSlot = Root->AddChildToCanvas(InteractionText);
		PromptCanvasSlot->SetAnchors(FAnchors(0.5f, 1.0f));
		PromptCanvasSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		PromptCanvasSlot->SetPosition(FVector2D(0.0f, -120.0f));
		PromptCanvasSlot->SetAutoSize(true);
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

	Super::NativeDestruct();
}

void URANDHUDWidget::RefreshAll()
{
	if (UHealthComponent* Health = BoundHealth.Get())
	{
		if (HealthBar)
		{
			HealthBar->SetPercent(Health->GetHealthPercent());
		}
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
}

// --- Delegate handlers ------------------------------------------------------

void URANDHUDWidget::HandleHealthChanged(float /*NewHealth*/, float /*Delta*/)
{
	if (HealthBar)
	{
		if (UHealthComponent* Health = BoundHealth.Get())
		{
			HealthBar->SetPercent(Health->GetHealthPercent());
		}
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
