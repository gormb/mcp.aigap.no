#pragma once

#include "CoreMinimal.h"
#include "BookMediaTypes.h"
#include "BookTypes.h"
#include "MarkdownTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BookStateSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBookPositionChanged, const FBookPosition& /*Position*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBookModeChanged, EExperienceMode /*Mode*/);

/**
 * Owns the open book, the shared reading cursor and the experience mode.
 * Presentation and audio layers listen here instead of talking to each other.
 */
UCLASS()
class UEBOOK_API UBookStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void OpenBook(const FString& InBookPath, const FMarkdownDocument& InDocument, const FBookMediaManifest& InManifest);
	void CloseBook();

	UFUNCTION(BlueprintPure, Category = "Book")
	bool IsBookOpen() const { return bBookOpen; }

	UFUNCTION(BlueprintPure, Category = "Book")
	FString GetBookPath() const { return BookPath; }

	const FMarkdownDocument& GetDocument() const { return Document; }
	const FBookMediaManifest& GetManifest() const { return Manifest; }

	UFUNCTION(BlueprintPure, Category = "Book")
	FBookPosition GetPosition() const { return Position; }

	/** Jumps to a subchapter (also moves the page cursor to that subchapter's first page). */
	void SetSubChapter(int32 SubChapterIndex);
	void NextSubChapter();
	void PreviousSubChapter();

	UFUNCTION(BlueprintPure, Category = "Book")
	EExperienceMode GetMode() const { return Mode; }

	void SetMode(EExperienceMode NewMode);
	void CycleMode();
	void SetModeByName(const FString& ModeName);

	/** Selects by ladder index (0 = Link … 6 = Speculative) when that mode is available. */
	void SetModeByIndex(int32 Index);

	/** Number of modes in the ladder (M0..M6). */
	static int32 GetModeCount();

	/** Availability plus human text for every mode, in ladder order. */
	void GetModeInfos(TArray<FBookModeInfo>& OutInfos) const;

	bool IsModeAvailable(EExperienceMode InMode, FString& OutReason) const;

	UFUNCTION(BlueprintPure, Category = "Book")
	FString GetModeName() const;

	FString GetNarratorId() const { return NarratorId; }
	FString GetNarratorDisplayName() const;
	void CycleNarrator();
	void SetNarrator(const FString& InNarratorId);

	/** True when the current subchapter has a resolvable narration source. */
	bool HasNarrationForCurrentSubChapter() const;

	/** Current subchapter, or null when the book is closed or the index is out of range. */
	const FBookSubChapter* GetCurrentSubChapter() const;

	void SaveProgress();
	void LoadProgress();

	/** Forgets the saved cursor/mode for any book. */
	void ResetProgress();

	FOnBookPositionChanged OnPositionChanged;
	FOnBookModeChanged OnModeChanged;

	static FString GetSaveSlotName() { return TEXT("UeBookProgress"); }

private:
	void ApplyPosition(const FBookPosition& NewPosition);
	void BroadcastMode();

	FMarkdownDocument Document;
	FBookMediaManifest Manifest;
	FString BookPath;
	FString BookId;
	FBookPosition Position;
	EExperienceMode Mode = EExperienceMode::Codex;
	FString NarratorId;
	bool bBookOpen = false;
};
