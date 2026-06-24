// Copyright Fortitudo Studio. All Rights Reserved.

#include "DialogueComponent.h"
#include "TimeComponent.h"

URANDDialogueComponent::URANDDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDDialogueComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Don't leave the world clock frozen if we're torn down mid-conversation.
	if (IsDialogueActive())
	{
		SetGameTimePaused(false);
		CurrentLineIndex = INDEX_NONE;
	}

	Super::EndPlay(EndPlayReason);
}

void URANDDialogueComponent::StartDialogue()
{
	if (IsDialogueActive() || !HasDialogue())
	{
		return;
	}

	CurrentLineIndex = 0;
	SetGameTimePaused(true);
	OnDialogueStarted.Broadcast(DialogueLines[CurrentLineIndex]);
}

void URANDDialogueComponent::AdvanceDialogue(int32 ChoiceIndex)
{
	if (!IsDialogueActive())
	{
		return;
	}

	const FRANDDialogueLine& Current = DialogueLines[CurrentLineIndex];

	// Terminal line (no choices): advancing closes the conversation.
	if (Current.Choices.Num() == 0)
	{
		EndDialogue();
		return;
	}

	// An out-of-range choice is ignored (e.g. UI sent a stale index).
	if (!Current.Choices.IsValidIndex(ChoiceIndex))
	{
		return;
	}

	const int32 NextIndex = Current.Choices[ChoiceIndex].NextLineIndex;
	if (!DialogueLines.IsValidIndex(NextIndex))
	{
		// -1 or any invalid target ends the dialogue.
		EndDialogue();
		return;
	}

	CurrentLineIndex = NextIndex;
	OnDialogueAdvanced.Broadcast(DialogueLines[CurrentLineIndex]);
}

void URANDDialogueComponent::EndDialogue()
{
	if (!IsDialogueActive())
	{
		return;
	}

	CurrentLineIndex = INDEX_NONE;
	SetGameTimePaused(false);
	OnDialogueEnded.Broadcast();
}

bool URANDDialogueComponent::GetCurrentLine(FRANDDialogueLine& OutLine) const
{
	if (DialogueLines.IsValidIndex(CurrentLineIndex))
	{
		OutLine = DialogueLines[CurrentLineIndex];
		return true;
	}
	return false;
}

void URANDDialogueComponent::SetGameTimePaused(bool bPaused)
{
	if (!Time.IsValid())
	{
		Time = URANDTimeComponent::Get(this);
	}
	if (URANDTimeComponent* Clock = Time.Get())
	{
		Clock->SetPaused(bPaused);
	}
}
