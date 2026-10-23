#pragma once

#include "CoreMinimal.h"
#include "BookTypes.h"
#include "MarkdownTypes.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"

DECLARE_DELEGATE(FOnBookReaderClosed);
DECLARE_DELEGATE(FOnBookReaderAction);
DECLARE_DELEGATE_OneParam(FOnBookSubChapterChanged, int32 /*SubChapterIndex*/);

/** Clickable text that opens a URL in the platform browser. */
class SBookLink : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBookLink) {}
		SLATE_ARGUMENT(FText, Text)
		SLATE_ARGUMENT(FString, Url)
		SLATE_ARGUMENT(FSlateFontInfo, Font)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;

private:
	FString Url;
	TSharedPtr<STextBlock> Label;
};

/**
 * Fullscreen scrolling reader for a parsed markdown document.
 * Experience state is pushed in by the owner; the widget only reports user intent.
 */
class SBookReader : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBookReader) {}
		SLATE_ARGUMENT(FMarkdownDocument, Document)
		SLATE_EVENT(FOnBookReaderClosed, OnClosed)
		SLATE_EVENT(FOnBookReaderAction, OnCycleMode)
		SLATE_EVENT(FOnBookReaderAction, OnToggleNarration)
		SLATE_EVENT(FOnBookReaderAction, OnCycleNarrator)
		SLATE_EVENT(FOnBookReaderAction, OnOpenLink)
		SLATE_EVENT(FOnBookSubChapterChanged, OnSubChapterChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Presentation state pushed by the owner. */
	void SetMode(EExperienceMode InMode);
	void SetNarratorName(const FString& InNarratorName);
	void SetPosition(const FBookPosition& InPosition);

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TSharedRef<SWidget> MakeBlockWidget(const FMarkdownBlock& Block);
	void BuildBlocks();
	void JumpToSubChapter(int32 Direction);
	void ScrollToSubChapter(int32 SubChapterIndex);
	void ScrollToBlock(int32 BlockIndex);
	void UpdateStatus();

	FMarkdownDocument Document;
	FOnBookReaderClosed OnClosed;
	FOnBookReaderAction OnCycleMode;
	FOnBookReaderAction OnToggleNarration;
	FOnBookReaderAction OnCycleNarrator;
	FOnBookReaderAction OnOpenLink;
	FOnBookSubChapterChanged OnSubChapterChanged;

	EExperienceMode Mode = EExperienceMode::Codex;
	FString NarratorName;
	int32 CurrentSubChapterIndex = 0;
	int32 CurrentPageNumber = 0;

	TSharedPtr<SScrollBox> ScrollBox;
	TSharedPtr<STextBlock> StatusLabel;
	TArray<TSharedPtr<SWidget>> BlockWidgets;
	int32 FramesSinceConstruct = 0;
};
