// Copyright Fortitudo Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueComponent.generated.h"

class URANDTimeComponent;

/**
 * FRANDDialogueChoice — one selectable response on a dialogue line. Picking it
 * jumps the conversation to NextLineIndex; a NextLineIndex that isn't a valid
 * line (e.g. -1) ends the conversation.
 */
USTRUCT(BlueprintType)
struct FRANDDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText ChoiceText;

	/** Index of the line this choice leads to. -1 (or any invalid index) ends the dialogue. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 NextLineIndex = -1;
};

/**
 * FRANDDialogueLine — a single node in a conversation tree: who speaks, what
 * they say, and the choices offered. A line with no choices is terminal —
 * advancing from it ends the conversation. A linear "Continue" is just a single
 * choice pointing at the next line.
 */
USTRUCT(BlueprintType)
struct FRANDDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText Line;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FRANDDialogueChoice> Choices;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, const FRANDDialogueLine&, Line);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueAdvanced, const FRANDDialogueLine&, Line);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);

/**
 * URANDDialogueComponent — drives a branching conversation authored as a flat
 * array of FRANDDialogueLine (entry point is line 0). Attaches to an NPC; the
 * NPC's OnInteract calls StartDialogue. Game time is paused for the duration so
 * the world holds still while the player reads/chooses.
 *
 * A UI layer subscribes to the delegates to render the current speaker, line,
 * and choices; it calls AdvanceDialogue with the chosen index.
 */
UCLASS(ClassGroup = "RAND", meta = (BlueprintSpawnableComponent))
class RAND_API URANDDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URANDDialogueComponent();

	/** The conversation tree. Line 0 is the entry point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FRANDDialogueLine> DialogueLines;

	// --- Flow ---------------------------------------------------------------

	/** Begins at line 0, pauses game time, and broadcasts OnDialogueStarted. No-op if already active or empty. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue();

	/**
	 * Follows the chosen choice from the current line. Ends the dialogue if the
	 * current line is terminal or the target index is invalid; otherwise moves
	 * to the next line and broadcasts OnDialogueAdvanced.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AdvanceDialogue(int32 ChoiceIndex);

	/** Ends the dialogue, resumes game time, and broadcasts OnDialogueEnded. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	// --- Queries ------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsDialogueActive() const { return CurrentLineIndex != INDEX_NONE; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool HasDialogue() const { return DialogueLines.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	int32 GetCurrentLineIndex() const { return CurrentLineIndex; }

	/** Copies out the current line; returns false when no dialogue is active. */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool GetCurrentLine(FRANDDialogueLine& OutLine) const;

	// --- Delegates ----------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueStarted OnDialogueStarted;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueAdvanced OnDialogueAdvanced;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueEnded OnDialogueEnded;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Index into DialogueLines, or INDEX_NONE when no conversation is active. */
	int32 CurrentLineIndex = INDEX_NONE;

	TWeakObjectPtr<URANDTimeComponent> Time;

	/** Pauses/resumes the game clock (resolved lazily). */
	void SetGameTimePaused(bool bPaused);
};
