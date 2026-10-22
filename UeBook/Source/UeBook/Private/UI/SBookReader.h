#pragma once

#include "CoreMinimal.h"
#include "MarkdownTypes.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"

DECLARE_DELEGATE(FOnBookReaderClosed);

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

/** Fullscreen scrolling reader for a parsed markdown document. */
class SBookReader : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBookReader) {}
		SLATE_ARGUMENT(FMarkdownDocument, Document)
		SLATE_EVENT(FOnBookReaderClosed, OnClosed)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Moves the view to the next (1) or previous (-1) heading. */
	void JumpToChapter(int32 Direction);

private:
	TSharedRef<SWidget> MakeBlockWidget(const FMarkdownBlock& Block);
	void BuildBlocks();

FMarkdownDocument Document;
FOnBookReaderClosed OnClosed;

TSharedPtr<SScrollBox> ScrollBox;
TArray<TSharedPtr<SWidget>> BlockWidgets;
TArray<int32> HeadingBlockIndices;
int32 CurrentHeadingIndex = INDEX_NONE;
int32 FramesSinceConstruct = 0;
};
