#include "UI/SBookReader.h"

#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformProcess.h"
#include "Input/Events.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "UI/BookFonts.h"
#include "UObject/Class.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FLinearColor BodyColor(0.84f, 0.84f, 0.87f);
	const FLinearColor HeadingColor(1.0f, 0.94f, 0.86f);
	const FLinearColor MutedColor(0.42f, 0.43f, 0.48f);
	const FLinearColor LinkColor(0.55f, 0.75f, 0.95f);
	const FLinearColor AccentColor(0.14f, 0.29f, 0.42f);
	const FLinearColor ChipColor(0.09f, 0.09f, 0.11f);

	FSlateBrush MakeFlatBrush(const FLinearColor& Color)
	{
		FSlateBrush Brush = *FCoreStyle::Get().GetBrush("WhiteBrush");
		Brush.TintColor = FSlateColor(Color);
		Brush.DrawAs = ESlateBrushDrawType::Box;
		return Brush;
	}

	FButtonStyle MakeChipStyle(const FLinearColor& Normal, const FLinearColor& Hover)
	{
		FButtonStyle Style;
		Style.SetNormal(MakeFlatBrush(Normal));
		Style.SetHovered(MakeFlatBrush(Hover));
		Style.SetPressed(MakeFlatBrush(Hover));
		Style.SetDisabled(MakeFlatBrush(FLinearColor(0.06f, 0.06f, 0.07f)));
		Style.SetNormalPadding(FMargin(0.0f));
		Style.SetPressedPadding(FMargin(0.0f));
		return Style;
	}

	const FButtonStyle& GetChipStyle(bool bActive)
	{
		static const FButtonStyle ActiveStyle = MakeChipStyle(AccentColor, FLinearColor(0.19f, 0.37f, 0.53f));
		static const FButtonStyle IdleStyle = MakeChipStyle(ChipColor, FLinearColor(0.17f, 0.17f, 0.21f));
		return bActive ? ActiveStyle : IdleStyle;
	}

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
	];
}

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
	OnCycleMode = InArgs._OnCycleMode;
	OnToggleNarration = InArgs._OnToggleNarration;
	OnCycleNarrator = InArgs._OnCycleNarrator;
	OnOpenLink = InArgs._OnOpenLink;
	OnSubChapterChanged = InArgs._OnSubChapterChanged;
	OnModeSelected = InArgs._OnModeSelected;

	const FString Title = Document.Title.IsEmpty() ? TEXT("Book") : Document.Title;

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
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
				.Padding(FMargin(0.0f, 40.0f, 0.0f, 10.0f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Title))
						.Font(BookFonts::Get(32, true))
						.ColorAndOpacity(HeadingColor)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
					[
						SAssignNew(StatusLabel, STextBlock)
						.Font(BookFonts::Get(14))
						.ColorAndOpacity(MutedColor)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
					[
						SAssignNew(ModeBar, SHorizontalBox)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Tab  modes      M  cycle      Space  listen      N  narrator      L  link      Page Up / Page Down  chapter      Esc  close")))
						.Font(BookFonts::Get(12))
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
		]
		+ SOverlay::Slot()
		[
			SAssignNew(PickerLayer, SBorder)
			.Visibility(this, &SBookReader::GetPickerVisibility)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.72f))
			.Padding(FMargin(0.0f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(640.0f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.06f, 0.99f))
					.Padding(FMargin(28.0f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Experience modes")))
							.Font(BookFonts::Get(26, true))
							.ColorAndOpacity(HeadingColor)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 6.0f, 0.0f, 16.0f))
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Up / Down  select       Enter  apply      1-7  direct      Esc  close")))
							.Font(BookFonts::Get(12))
							.ColorAndOpacity(MutedColor)
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						[
							SAssignNew(PickerList, SVerticalBox)
						]
					]
				]
			]
		]
	];

	BuildBlocks();
	UpdateStatus();
	RebuildModeBar();
}

void SBookReader::SetMode(EExperienceMode InMode)
{
	Mode = InMode;
	RebuildModeBar();
	UpdateStatus();
}

void SBookReader::SetModeInfos(const TArray<FBookModeInfo>& InModeInfos)
{
	ModeInfos = InModeInfos;
	RebuildModeBar();
	if (bPickerOpen)
	{
		RebuildPicker();
	}
}

EVisibility SBookReader::GetPickerVisibility() const
{
	return bPickerOpen ? EVisibility::Visible : EVisibility::Collapsed;
}

void SBookReader::RebuildModeBar()
{
	if (!ModeBar.IsValid())
	{
		return;
	}

	ModeBar->ClearChildren();

	for (int32 Index = 0; Index < ModeInfos.Num(); ++Index)
	{
		const FBookModeInfo& Info = ModeInfos[Index];
		const bool bActive = Info.Mode == Mode;
		const FString Tip = Info.bAvailable
			? FString::Printf(TEXT("%s - %s"), *Info.Label, *Info.Description)
			: FString::Printf(TEXT("%s - %s"), *Info.Label, *Info.UnavailableReason);

		ModeBar->AddSlot()
			.AutoWidth()
			.Padding(FMargin(0.0f, 0.0f, 6.0f, 0.0f))
			[
				SNew(SButton)
				.ButtonStyle(&GetChipStyle(bActive))
				.ContentPadding(FMargin(10.0f, 5.0f))
				.IsEnabled(Info.bAvailable)
				.RenderOpacity(Info.bAvailable ? 1.0f : 0.45f)
				.ToolTipText(FText::FromString(Tip))
				.OnClicked_Lambda([this, Index]()
				{
					SelectModeAt(Index);
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("M%d  %s"), Index, *Info.Label)))
					.Font(BookFonts::Get(13, bActive))
					.ColorAndOpacity(bActive ? FLinearColor(1.0f, 0.97f, 0.9f) : FLinearColor(0.72f, 0.75f, 0.8f))
				]
			];
	}
}

void SBookReader::RebuildPicker()
{
	if (!PickerList.IsValid())
	{
		return;
	}

	PickerList->ClearChildren();

	for (int32 Index = 0; Index < ModeInfos.Num(); ++Index)
	{
		const FBookModeInfo& Info = ModeInfos[Index];
		const bool bSelected = Index == PickerIndex;
		const bool bActive = Info.Mode == Mode;

		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);

		Row->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("M%d"), Index)))
				.Font(BookFonts::Get(12, true))
				.ColorAndOpacity(MutedColor)
			];

		Row->AddSlot()
			.AutoWidth()
			.Padding(FMargin(12.0f, 0.0f, 0.0f, 0.0f))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Info.Label))
				.Font(BookFonts::Get(19, bActive))
				.ColorAndOpacity(Info.bAvailable ? HeadingColor : MutedColor)
			];

		Row->AddSlot()
			.FillWidth(1.0f)
			.Padding(FMargin(16.0f, 0.0f, 0.0f, 0.0f))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Info.bAvailable ? Info.Description : Info.UnavailableReason))
				.Font(BookFonts::Get(13))
				.ColorAndOpacity(Info.bAvailable ? MutedColor : FLinearColor(0.62f, 0.40f, 0.36f))
				.AutoWrapText(true)
			];

		if (bActive)
		{
			Row->AddSlot()
				.AutoWidth()
				.Padding(FMargin(12.0f, 0.0f, 0.0f, 0.0f))
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("active")))
					.Font(BookFonts::Get(11, true))
					.ColorAndOpacity(FLinearColor(0.62f, 0.82f, 0.62f))
				];
		}

		PickerList->AddSlot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 2.0f))
			[
				SNew(SButton)
				.ButtonStyle(&GetChipStyle(bSelected))
				.ContentPadding(FMargin(14.0f, 10.0f))
				.IsEnabled(Info.bAvailable)
				.RenderOpacity(Info.bAvailable ? 1.0f : 0.5f)
				.ToolTipText(FText::FromString(Info.bAvailable ? Info.Description : Info.UnavailableReason))
				.OnClicked_Lambda([this, Index]()
				{
					SelectModeAt(Index);
					return FReply::Handled();
				})
				[
					Row
				]
			];
	}
}

void SBookReader::TogglePicker()
{
	ShowModePicker(!bPickerOpen);
}

void SBookReader::ShowModePicker(bool bShow)
{
	bPickerOpen = bShow;

	if (bPickerOpen)
	{
		PickerIndex = FMath::Clamp(static_cast<int32>(Mode), 0, FMath::Max(ModeInfos.Num() - 1, 0));
		RebuildPicker();
	}
}

void SBookReader::SelectModeAt(int32 Index)
{
	if (!ModeInfos.IsValidIndex(Index) || !ModeInfos[Index].bAvailable)
	{
		return;
	}

	bPickerOpen = false;
	OnModeSelected.ExecuteIfBound(ModeInfos[Index].Mode);
}

void SBookReader::MovePickerSelection(int32 Delta)
{
	if (ModeInfos.Num() == 0)
	{
		return;
	}

	int32 Candidate = PickerIndex;
	while (true)
	{
		Candidate = FMath::Clamp(Candidate + Delta, 0, ModeInfos.Num() - 1);

		if (ModeInfos[Candidate].bAvailable)
		{
			break;
		}

		if ((Delta > 0 && Candidate >= ModeInfos.Num() - 1) || (Delta < 0 && Candidate <= 0))
		{
			break;
		}
	}

	PickerIndex = Candidate;
	RebuildPicker();
}

void SBookReader::SetNarratorName(const FString& InNarratorName)
{
	NarratorName = InNarratorName;
	UpdateStatus();
}

void SBookReader::SetPosition(const FBookPosition& InPosition)
{
	CurrentPageNumber = InPosition.PageNumber;

	if (Document.SubChapters.IsValidIndex(InPosition.SubChapterIndex)
		&& InPosition.SubChapterIndex != CurrentSubChapterIndex)
	{
		CurrentSubChapterIndex = InPosition.SubChapterIndex;
		ScrollToSubChapter(CurrentSubChapterIndex);
	}

	UpdateStatus();
}

void SBookReader::UpdateStatus()
{
	if (!StatusLabel.IsValid())
	{
		return;
	}

	const FString ModeName = StaticEnum<EExperienceMode>()->GetNameStringByValue(static_cast<int64>(Mode));

	FString Status = FString::Printf(TEXT("%s   ·   %d subchapters   ·   page %d"),
		*ModeName, Document.SubChapters.Num(), FMath::Max(CurrentPageNumber, 0));

	if (!NarratorName.IsEmpty())
	{
		Status += FString::Printf(TEXT("   ·   narrator: %s"), *NarratorName);
	}

	StatusLabel->SetText(FText::FromString(Status));
}

void SBookReader::BuildBlocks()
{
	if (!ScrollBox.IsValid())
	{
		return;
	}

	ScrollBox->ClearChildren();
	BlockWidgets.Reset();

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
	}
}

TSharedRef<SWidget> SBookReader::MakeBlockWidget(const FMarkdownBlock& Block)
{
	switch (Block.Type)
	{
	case EMarkdownBlockType::Heading:
	{
		static const int32 HeadingSizes[] = { 40, 32, 26, 21, 18, 16 };
		const int32 Size = HeadingSizes[FMath::Clamp(Block.Level - 1, 0, 5)];
		const float TopPadding = Block.Level <= 2 ? 30.0f : 20.0f;

		return SNew(SBox)
			.Padding(FMargin(0.0f, TopPadding, 0.0f, 8.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Block.Text))
				.Font(BookFonts::Get(Size, true))
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
				.Font(BookFonts::Get(11))
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(MutedColor)
			];

	case EMarkdownBlockType::Quote:
		return WrapText(Block.Text, BookFonts::Get(19, false, true), FLinearColor(0.68f, 0.70f, 0.76f), FMargin(20.0f, 10.0f, 0.0f, 10.0f));

	case EMarkdownBlockType::Bullet:
		return WrapText(FString::Printf(TEXT("•   %s"), *Block.Text), BookFonts::Get(19), BodyColor, FMargin(24.0f, 3.0f));

	case EMarkdownBlockType::Numbered:
		return WrapText(Block.Text, BookFonts::Get(19), BodyColor, FMargin(24.0f, 3.0f));

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
					.Font(BookFonts::Get(14))
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
		TSharedRef<SWidget> Body = Block.Url.IsEmpty()
			? WrapText(Block.Text, BookFonts::Get(16), LinkColor, FMargin(0.0f, 0.0f))
			: StaticCastSharedRef<SWidget>(SNew(SBookLink)
				.Text(FText::FromString(Block.Text))
				.Url(Block.Url)
				.Font(BookFonts::Get(16)));

		return SNew(SBox)
			.Padding(FMargin(0.0f, 10.0f))
			[
				SNew(SHorizontalBox)
				// EB Garamond has no music note, so draw the marker with the engine font.
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("♪")))
					.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 15))
					.ColorAndOpacity(LinkColor)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(FMargin(10.0f, 0.0f, 0.0f, 0.0f))
				.VAlign(VAlign_Center)
				[
					Body
				]
			];
	}

	case EMarkdownBlockType::Paragraph:
	default:
		return WrapText(Block.Text, BookFonts::Get(20), BodyColor, FMargin(0.0f, 6.0f));
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

	if (Key == EKeys::Tab)
	{
		TogglePicker();
		return FReply::Handled();
	}

	if (bPickerOpen)
	{
		if (Key == EKeys::Escape)
		{
			bPickerOpen = false;
			return FReply::Handled();
		}

		if (Key == EKeys::Up || Key == EKeys::Down)
		{
			MovePickerSelection(Key == EKeys::Down ? 1 : -1);
			return FReply::Handled();
		}

		if (Key == EKeys::Enter)
		{
			SelectModeAt(PickerIndex);
			return FReply::Handled();
		}
	}

	// 1-7 jump straight to a mode in the ladder.
	static const FKey ModeKeys[] = {
		EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six, EKeys::Seven
	};
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(ModeKeys); ++Index)
	{
		if (Key == ModeKeys[Index])
		{
			SelectModeAt(Index);
			return FReply::Handled();
		}
	}

	if (Key == EKeys::Escape)
	{
		OnClosed.ExecuteIfBound();
		return FReply::Handled();
	}

	if (Key == EKeys::M)
	{
		OnCycleMode.ExecuteIfBound();
		return FReply::Handled();
	}

	if (Key == EKeys::SpaceBar)
	{
		OnToggleNarration.ExecuteIfBound();
		return FReply::Handled();
	}

	if (Key == EKeys::N)
	{
		OnCycleNarrator.ExecuteIfBound();
		return FReply::Handled();
	}

	if (Key == EKeys::L)
	{
		OnOpenLink.ExecuteIfBound();
		return FReply::Handled();
	}

	if (Key == EKeys::PageDown)
	{
		JumpToSubChapter(1);
		return FReply::Handled();
	}

	if (Key == EKeys::PageUp)
	{
		JumpToSubChapter(-1);
		return FReply::Handled();
	}

	if (Key == EKeys::Home && ScrollBox.IsValid())
	{
		ScrollBox->ScrollToStart();
		CurrentSubChapterIndex = 0;
		CurrentPageNumber = Document.SubChapters.Num() > 0 ? Document.SubChapters[0].FirstPage : 0;
		UpdateStatus();
		OnSubChapterChanged.ExecuteIfBound(0);
		return FReply::Handled();
	}

	if (Key == EKeys::End && ScrollBox.IsValid())
	{
		ScrollBox->ScrollToEnd();
		if (Document.SubChapters.Num() > 0)
		{
			const int32 LastIndex = Document.SubChapters.Num() - 1;
			CurrentSubChapterIndex = LastIndex;
			CurrentPageNumber = Document.SubChapters[LastIndex].FirstPage;
			UpdateStatus();
			OnSubChapterChanged.ExecuteIfBound(LastIndex);
		}
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SBookReader::JumpToSubChapter(int32 Direction)
{
	if (!ScrollBox.IsValid() || Document.SubChapters.Num() == 0)
	{
		return;
	}

	const int32 NewIndex = FMath::Clamp(CurrentSubChapterIndex + Direction, 0, Document.SubChapters.Num() - 1);
	if (NewIndex == CurrentSubChapterIndex)
	{
		return;
	}

	CurrentSubChapterIndex = NewIndex;
	CurrentPageNumber = Document.SubChapters[NewIndex].FirstPage;
	UpdateStatus();
	ScrollToSubChapter(NewIndex);
	OnSubChapterChanged.ExecuteIfBound(NewIndex);
}

void SBookReader::ScrollToSubChapter(int32 SubChapterIndex)
{
	if (!Document.SubChapters.IsValidIndex(SubChapterIndex))
	{
		return;
	}

	ScrollToBlock(Document.SubChapters[SubChapterIndex].FirstBlock);
}

void SBookReader::ScrollToBlock(int32 BlockIndex)
{
	if (!ScrollBox.IsValid() || !BlockWidgets.IsValidIndex(BlockIndex))
	{
		return;
	}

	ScrollBox->ScrollDescendantIntoView(BlockWidgets[BlockIndex], true);
}
