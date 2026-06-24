// Copyright Fortitudo Studio. All Rights Reserved.

#include "InteractionComponent.h"
#include "IInteractable.h"

#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz — cheap enough for proximity checks.
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Build a proximity sphere attached to the owner root.
	ProximitySphere = NewObject<USphereComponent>(GetOwner(), TEXT("InteractionSphere"));
	ProximitySphere->SetSphereRadius(InteractionRadius);
	ProximitySphere->SetCollisionProfileName(TEXT("OverlapAll"));
	ProximitySphere->SetGenerateOverlapEvents(true);
	ProximitySphere->RegisterComponent();
	ProximitySphere->AttachToComponent(GetOwner()->GetRootComponent(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// Wire IA_Interact (E key) through the character's Enhanced Input setup.
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar)
	{
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// Create a dedicated mapping context for interaction so it can be
			// pushed/popped independently (e.g. suppressed during cutscenes).
			UInputMappingContext* InteractContext =
				NewObject<UInputMappingContext>(this, TEXT("InteractMappingContext"));

			UInputAction* InteractAction =
				NewObject<UInputAction>(this, TEXT("IA_Interact"));
			InteractAction->ValueType = EInputActionValueType::Boolean;

			InteractContext->MapKey(InteractAction, EKeys::E);

			Subsystem->AddMappingContext(InteractContext, 1); // Priority 1 — above character movement.

			if (UEnhancedInputComponent* EIC =
				Cast<UEnhancedInputComponent>(OwnerChar->InputComponent))
			{
				EIC->BindAction(InteractAction, ETriggerEvent::Started, this,
					&UInteractionComponent::TryInteract);
			}
		}
	}
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Best = FindBestTarget();
	if (Best != CurrentTarget.Get())
	{
		CurrentTarget = Best;
		OnTargetChanged.Broadcast(Best);
	}
}

FText UInteractionComponent::GetPromptText() const
{
	if (AActor* Target = CurrentTarget.Get())
	{
		if (IInteractable* Interactable = Cast<IInteractable>(Target))
		{
			return IInteractable::Execute_GetInteractionPrompt(Target);
		}
	}
	return FText::GetEmpty();
}

void UInteractionComponent::TryInteract()
{
	AActor* Target = CurrentTarget.Get();
	if (!Target)
	{
		return;
	}

	if (Target->Implements<UInteractable>())
	{
		IInteractable::Execute_OnInteract(Target, GetOwner());
	}
}

AActor* UInteractionComponent::FindBestTarget() const
{
	if (!ProximitySphere)
	{
		return nullptr;
	}

	TArray<AActor*> Overlapping;
	ProximitySphere->GetOverlappingActors(Overlapping);

	AActor* Best = nullptr;
	float BestDistSq = FLT_MAX;
	const FVector OwnerLoc = GetOwner()->GetActorLocation();

	for (AActor* Actor : Overlapping)
	{
		if (!Actor || Actor == GetOwner())
		{
			continue;
		}
		if (!Actor->Implements<UInteractable>())
		{
			continue;
		}
		if (!IInteractable::Execute_CanInteract(Actor, GetOwner()))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(OwnerLoc, Actor->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Actor;
		}
	}

	return Best;
}
