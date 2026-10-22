#include "UI/SBookReader.h"

#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformProcess.h"
#include "Input/Events.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	FSlateFontInfo BookFont(int32 Size, bool bBold = false, bool bItalic = false)
	{
		FName Typeface = TEXT("Regular");
		if (bBold && bItalic)
		{
			Typeface = TEXT("BoldItalic");
		}
		else if (bBold)
		{
			Typeface = TEXT("Bold");
		}
		else if (bItalic)
		{
			Typeface = TEXT("Italic");
		}

		return FCoreStyle::GetDefaultFontStyle(Typeface, Size);
	}

	const FLinearColor BodyColor(0.84f, 0.84f, 0.87f);
	const FLinearColor HeadingColor(1.0f, 0.94f, 0.86f);
	const FLinearColor MutedColor(0.42f, 0.43f, 0.48f);
	const FLinearColor LinkColor(0.55f, 0.75f, 0.95f);

	TSharedRef<SWidget> WrapText(const FString& Text, const FSlateFontInfo& Font, const FLinearColor& Color, const FMargin& Padding)
	{
		return SNew(SBox)
			.Padding(Padding)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Text))
				.Font(Font)
				.ColorAndOpacity(Color)
				.LineHeightPercentage(1.35f)
				.AutoWrapText(true)
			];
	}
}

void SBookLink::Construct(const FArguments& InArgs)
{
	Url = InArgs._Url;

	ChildSlot
	[
		SAssignNew(Label, STextBlock)
		.Text(InArgs._Text)
		.Font(InArgs._Font)
		.ColorAndOpacity(LinkColor)
		.AutoWrapText(true)
	];}

FReply SBookLink::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && !Url.IsEmpty())
	{
		FPlatformProcess::LaunchURL(*Url, nullptr, nullptr);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void SBookLink::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (Label.IsValid())
	{
		Label->SetColorAndOpacity(FLinearColor(0.78f, 0.9f, 1.0f));
	}
}

void SBookLink::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	if (Label.IsValid())
	{
		Label->SetColorAndOpacity(LinkColor);
	}
}

FCursorReply SBookLink::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	return FCursorReply::Cursor(EMouseCursor::Hand);
}

void SBookReader::Construct(const FArguments& InArgs)
{
	Document = InArgs._Document;
	OnClosed = InArgs._OnClosed;

	const FString Title = Document.Title.IsEmpty() ? TEXT("Book") : Document.Title;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 0.98f))
		.Padding(FMargin(0.0f))
		[
			// Proportional spacers keep the reading column centred at any resolution or DPI scale.
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SSpacer)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(4.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 44.0f, 0.0f, 10.0f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Title))
						.Font(BookFont(30, true))
						.ColorAndOpacity(HeadingColor)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Page Up / Page Down   chapters        Mouse wheel / drag   scroll        Home / End        Esc   close")))
						.Font(BookFont(12))
						.ColorAndOpacity(MutedColor)
					]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(FMargin(0.0f, 0.0f, 16.0f, 32.0f))
				[
					SAssignNew(ScrollBox, SScrollBox)
					.ScrollBarVisibility(EVisibility::Visible)
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SSpacer)
			]
		]
	];

	BuildBlocks();
}

void SBookReader::BuildBlocks()
{
	if (!ScrollBox.IsValid())
	{
		return;
	}

	ScrollBox->ClearChildren();
	BlockWidgets.Reset();
	HeadingBlockIndices.Reset();

	for (int32 Index = 0; Index < Document.Blocks.Num(); ++Index)
	{
		const FMarkdownBlock& Block = Document.Blocks[Index];

		// The header already shows the document title, so skip the identical first heading.
		const bool bRepeatOfTitle = Block.Type == EMarkdownBlockType::Heading
			&& Block.Level == 1
			&& Block.Text.Equals(Document.Title, ESearchCase::CaseSensitive);

		TSharedRef<SWidget> Row = SNew(SSpacer);
		if (!bRepeatOfTitle)
		{
			Row = MakeBlockWidget(Block);
		}

		ScrollBox->AddSlot()
			.HAlign(HAlign_Fill)
			.Padding(FMargin(0.0f, 2.0f))
			[
				Row
			];

		BlockWidgets.Add(Row);

		if (!bRepeatOfTitle && Block.Type == EMarkdownBlockType::Heading)
		{
			HeadingBlockIndices.Add(Index);
		}
	}

	CurrentHeadingIndex = HeadingBlockIndices.Num() > 0 ? 0 : INDEX_NONE;
}

TSharedRef<SWidget> SBookReader::MakeBlockWidget(const FMarkdownBlock& Block)
{
	switch (Block.Type)
	{
	case EMarkdownBlockType::Heading:
	{
		static const int32 HeadingSizes[] = { 40, 30, 24, 20, 18, 16 };
		const int32 Size = HeadingSizes[FMath::Clamp(Block.Level - 1, 0, 5)];
		const float TopPadding = Block.Level <= 2 ? 30.0f : 20.0f;

		return SNew(SBox)
			.Padding(FMargin(0.0f, TopPadding, 0.0f, 8.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Block.Text))
				.Font(BookFont(Size, true))
				.ColorAndOpacity(HeadingColor)
				.LineHeightPercentage(1.15f)
				.AutoWrapText(true)
			];
	}

	case EMarkdownBlockType::PageBreak:
		return SNew(SBox)
			.Padding(FMargin(0.0f, 12.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("·  %s  ·"), *Block.Text)))
				.Font(BookFont(11))
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(MutedColor)
			];

	case EMarkdownBlockType::Quote:
		return WrapText(Block.Text, BookFont(17, false, true), FLinearColor(0.68f, 0.70f, 0.76f), FMargin(20.0f, 10.0f, 0.0f, 10.0f));

	case EMarkdownBlockType::Bullet:
		return WrapText(FString::Printf(TEXT("•   %s"), *Block.Text), BookFont(17), BodyColor, FMargin(24.0f, 3.0f));

	case EMarkdownBlockType::Numbered:
		return WrapText(Block.Text, BookFont(17), BodyColor, FMargin(24.0f, 3.0f));

	case EMarkdownBlockType::Code:
		return SNew(SBox)
			.Padding(FMargin(0.0f, 10.0f))
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.09f, 0.09f, 0.11f))
				.Padding(FMargin(14.0f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(Block.Text))
					.Font(BookFont(14))
					.ColorAndOpacity(FLinearColor(0.82f, 0.86f, 0.82f))
				]
			];

	case EMarkdownBlockType::Rule:
		return SNew(SBox)
			.Padding(FMargin(0.0f, 18.0f))
			.HAlign(HAlign_Center)
			[
				SNew(SBox)
				.HeightOverride(1.0f)
				.WidthOverride(200.0f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.30f, 0.31f, 0.36f))
				]
			];

	case EMarkdownBlockType::MusicLink:
	{
		const FString Label = FString::Printf(TEXT("♪   %s"), *Block.Text);
		TSharedRef<SWidget> Link = Block.Url.IsEmpty()
			? WrapText(Label, BookFont(15), LinkColor, FMargin(0.0f, 0.0f))
			: StaticCastSharedRef<SWidget>(SNew(SBookLink)
				.Text(FText::FromString(Label))
				.Url(Block.Url)
				.Font(BookFont(15)));

		return SNew(SBox)
			.Padding(FMargin(0.0f, 10.0f))
			[
				Link
			];
	}

	case EMarkdownBlockType::Paragraph:
	default:
		return WrapText(Block.Text, BookFont(18), BodyColor, FMargin(0.0f, 6.0f));
	}
}

void SBookReader::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// The game viewport takes focus while it initialises, so claim it once the reader is on screen.
	if (++FramesSinceConstruct == 5 && !HasAnyUserFocus() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetAllUserFocus(SharedThis(this), EFocusCause::SetDirectly);
	}

	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

FReply SBookReader::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	UE_LOG(LogTemp, Warning, TEXT("Reader key: %s"), *Key.ToString());

	if (Key == EKeys::Escape)
	{
		OnClosed.ExecuteIfBound();
		return FReply::Handled();
	}

	if (Key == EKeys::PageDown)
	{
		JumpToChapter(1);
		return FReply::Handled();
	}

	if (Key == EKeys::PageUp)
	{
		JumpToChapter(-1);
		return FReply::Handled();
	}

	if (Key == EKeys::Home && ScrollBox.IsValid())
	{
		ScrollBox->ScrollToStart();
		CurrentHeadingIndex = HeadingBlockIndices.Num() > 0 ? 0 : INDEX_NONE;
		return FReply::Handled();
	}

	if (Key == EKeys::End && ScrollBox.IsValid())
	{
		ScrollBox->ScrollToEnd();
		CurrentHeadingIndex = HeadingBlockIndices.Num() - 1;
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SBookReader::JumpToChapter(int32 Direction)
{
	if (!ScrollBox.IsValid() || HeadingBlockIndices.Num() == 0)
	{
		return;
	}

	CurrentHeadingIndex = FMath::Clamp(CurrentHeadingIndex + Direction, 0, HeadingBlockIndices.Num() - 1);

	const int32 BlockIndex = HeadingBlockIndices[CurrentHeadingIndex];
	if (BlockWidgets.IsValidIndex(BlockIndex))
	{
		ScrollBox->ScrollDescendantIntoView(BlockWidgets[BlockIndex], true);
	}
}
