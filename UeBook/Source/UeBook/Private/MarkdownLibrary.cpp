#include "MarkdownLibrary.h"

#include "HAL/FileManager.h"
#include "Misc/Char.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	FString MakeSlug(const FString& Title, int32 Index)
	{
		FString Slug;
		for (const TCHAR Char : Title)
		{
			if (FChar::IsAlnum(Char))
			{
				Slug.AppendChar(FChar::ToLower(Char));
			}
			else if (!Slug.IsEmpty() && !Slug.EndsWith(TEXT("-")))
			{
				Slug.AppendChar(TEXT('-'));
			}
		}

		while (Slug.EndsWith(TEXT("-")))
		{
			Slug.LeftChopInline(1);
		}

		if (Slug.IsEmpty())
		{
			Slug = TEXT("section");
		}

		return FString::Printf(TEXT("%02d-%s"), Index, *Slug);
	}

	int32 ParsePageNumber(const FString& Text)
	{
		return FCString::Atoi(*Text.Mid(2).TrimStartAndEnd());
	}

	/** Replaces [label](url) and ![alt](url) with their visible text and reports the first target. */
	FString StripInlineLinks(const FString& In, FString& OutFirstUrl)
	{
		FString Result;
		Result.Reserve(In.Len());

		int32 Index = 0;
		while (Index < In.Len())
		{
			const int32 Open = In.Find(TEXT("["), ESearchCase::CaseSensitive, ESearchDir::FromStart, Index);
			if (Open == INDEX_NONE)
			{
				Result += In.Mid(Index);
				break;
			}

			const int32 LabelEnd = In.Find(TEXT("]("), ESearchCase::CaseSensitive, ESearchDir::FromStart, Open + 1);
			const int32 Close = (LabelEnd == INDEX_NONE)
				? INDEX_NONE
				: In.Find(TEXT(")"), ESearchCase::CaseSensitive, ESearchDir::FromStart, LabelEnd + 2);
			if (Close == INDEX_NONE)
			{
				Result += In.Mid(Index);
				break;
			}

			const bool bIsImage = Open > Index && In[Open - 1] == TEXT('!');
			Result += In.Mid(Index, (bIsImage ? Open - 1 : Open) - Index);
			Result += In.Mid(Open + 1, LabelEnd - (Open + 1));

			if (OutFirstUrl.IsEmpty())
			{
				OutFirstUrl = In.Mid(LabelEnd + 2, Close - (LabelEnd + 2));
			}

			Index = Close + 1;
		}

		return Result;
	}

	FString StripEmphasis(const FString& In)
	{
		FString Result = In;
		Result.ReplaceInline(TEXT("**"), TEXT(""));
		Result.ReplaceInline(TEXT("__"), TEXT(""));
		Result.ReplaceInline(TEXT("*"), TEXT(""));
		Result.ReplaceInline(TEXT("`"), TEXT(""));
		return Result;
	}

	void StripMusicNote(FString& Text);

	FMarkdownBlock MakeBlock(EMarkdownBlockType Type, const FString& Raw, int32 Line, int32 Level = 0)
	{
		FMarkdownBlock Block;
		Block.Type = Type;
		Block.Line = Line;
		Block.Level = Level;
		Block.Text = StripEmphasis(StripInlineLinks(Raw, Block.Url)).TrimStartAndEnd();
		StripMusicNote(Block.Text);
		return Block;
	}

	bool MatchHeading(const FString& Line, int32& OutLevel, FString& OutText)
	{
		int32 Hashes = 0;
		while (Hashes < Line.Len() && Line[Hashes] == TEXT('#'))
		{
			++Hashes;
		}

		if (Hashes < 1 || Hashes > 6 || Hashes >= Line.Len() || !FChar::IsWhitespace(Line[Hashes]))
		{
			return false;
		}

		OutLevel = Hashes;
		OutText = Line.Mid(Hashes).TrimStartAndEnd();
		return !OutText.IsEmpty();
	}

	/** The book marks pages as headings equal to "p. <number>". */
	bool IsPageMarker(const FString& Text)
	{
		if (!Text.StartsWith(TEXT("p."), ESearchCase::CaseSensitive))
		{
			return false;
		}

		const FString Digits = Text.Mid(2).TrimStartAndEnd();
		if (Digits.IsEmpty())
		{
			return false;
		}

		for (const TCHAR Char : Digits)
		{
			if (!FChar::IsDigit(Char))
			{
				return false;
			}
		}

		return true;
	}

	bool IsRule(const FString& Line)
	{
		if (Line.Len() < 3)
		{
			return false;
		}

		const TCHAR Marker = Line[0];
		if (Marker != TEXT('-') && Marker != TEXT('*') && Marker != TEXT('_'))
		{
			return false;
		}

		for (const TCHAR Char : Line)
		{
			if (Char != Marker && !FChar::IsWhitespace(Char))
			{
				return false;
			}
		}

		return true;
	}

	bool MatchBullet(const FString& Line, FString& OutText)
	{
		if (Line.Len() < 2)
		{
			return false;
		}

		const TCHAR Marker = Line[0];
		if ((Marker != TEXT('-') && Marker != TEXT('*') && Marker != TEXT('+')) || !FChar::IsWhitespace(Line[1]))
		{
			return false;
		}

		OutText = Line.Mid(1).TrimStartAndEnd();
		return !OutText.IsEmpty();
	}

	bool MatchNumbered(const FString& Line, FString& OutText)
	{
		int32 Digits = 0;
		while (Digits < Line.Len() && FChar::IsDigit(Line[Digits]))
		{
			++Digits;
		}

		if (Digits == 0 || Digits + 1 >= Line.Len())
		{
			return false;
		}

		const TCHAR Marker = Line[Digits];
		if ((Marker != TEXT('.') && Marker != TEXT(')')) || !FChar::IsWhitespace(Line[Digits + 1]))
		{
			return false;
		}

		OutText = Line.Mid(0, Digits + 1) + TEXT(" ") + Line.Mid(Digits + 1).TrimStartAndEnd();
		return true;
	}

	/** True when the line opens with the musical note emoji (U+1F3B5). */
	bool StartsWithMusicNote(const FString& Line)
	{
		return Line.Len() >= 2 && Line[0] == TCHAR(0xD83C) && Line[1] == TCHAR(0xDFB5);
	}

	/** Drops the leading note emoji so the reader can render a glyph its font actually has. */
	void StripMusicNote(FString& Text)
	{
		if (Text.Len() >= 2 && Text[0] == TCHAR(0xD83C) && Text[1] == TCHAR(0xDFB5))
		{
			Text.RightChopInline(2);
			Text.TrimStartInline();
		}
	}

	/** Derives narration subchapters and page ranges from the parsed blocks. */
	void BuildStructure(FMarkdownDocument& Document)
	{
		Document.Pages.Reset();
		Document.SubChapters.Reset();

		FPageData CurrentPage;
		bool bPageOpen = false;
		for (int32 Index = 0; Index < Document.Blocks.Num(); ++Index)
		{
			const FMarkdownBlock& Block = Document.Blocks[Index];
			if (Block.Type != EMarkdownBlockType::PageBreak)
			{
				continue;
			}

			if (bPageOpen)
			{
				CurrentPage.LastBlock = Index - 1;
				Document.Pages.Add(CurrentPage);
			}

			CurrentPage = FPageData();
			CurrentPage.Number = ParsePageNumber(Block.Text);
			CurrentPage.FirstBlock = Index;
			bPageOpen = true;
		}

		if (bPageOpen)
		{
			CurrentPage.LastBlock = Document.Blocks.Num() - 1;
			Document.Pages.Add(CurrentPage);
		}

		if (Document.Pages.Num() == 0 && Document.Blocks.Num() > 0)
		{
			FPageData Page;
			Page.Number = 1;
			Page.FirstBlock = 0;
			Page.LastBlock = Document.Blocks.Num() - 1;
			Document.Pages.Add(Page);
		}

		// `##` and `###` open a narration unit; deeper headings stay inside it.
		for (int32 Index = 0; Index < Document.Blocks.Num(); ++Index)
		{
			const FMarkdownBlock& Block = Document.Blocks[Index];
			if (Block.Type != EMarkdownBlockType::Heading || Block.Level < 2 || Block.Level > 3)
			{
				continue;
			}

			if (Document.SubChapters.Num() > 0)
			{
				Document.SubChapters.Last().LastBlock = Index - 1;
			}

			FBookSubChapter SubChapter;
			SubChapter.Title = Block.Text;
			SubChapter.HeadingLevel = Block.Level;
			SubChapter.FirstBlock = Index;
			SubChapter.Slug = MakeSlug(Block.Text, Document.SubChapters.Num() + 1);
			Document.SubChapters.Add(SubChapter);
		}

		if (Document.SubChapters.Num() > 0)
		{
			Document.SubChapters.Last().LastBlock = Document.Blocks.Num() - 1;
		}
		else if (Document.Blocks.Num() > 0)
		{
			FBookSubChapter SubChapter;
			SubChapter.Title = Document.Title.IsEmpty() ? TEXT("Book") : Document.Title;
			SubChapter.HeadingLevel = 1;
			SubChapter.FirstBlock = 0;
			SubChapter.LastBlock = Document.Blocks.Num() - 1;
			SubChapter.Slug = MakeSlug(SubChapter.Title, 1);
			Document.SubChapters.Add(SubChapter);
		}

		for (FPageData& Page : Document.Pages)
		{
			Page.SubChapterIndex = INDEX_NONE;
			for (int32 Index = 0; Index < Document.SubChapters.Num(); ++Index)
			{
				if (Page.FirstBlock >= Document.SubChapters[Index].FirstBlock
					&& Page.FirstBlock <= Document.SubChapters[Index].LastBlock)
				{
					Page.SubChapterIndex = Index;
					break;
				}
			}
		}

		for (int32 Index = 0; Index < Document.SubChapters.Num(); ++Index)
		{
			FBookSubChapter& SubChapter = Document.SubChapters[Index];
			SubChapter.FirstPage = 0;
			SubChapter.LastPage = 0;
			for (const FPageData& Page : Document.Pages)
			{
				if (Page.SubChapterIndex != Index)
				{
					continue;
				}

				if (SubChapter.FirstPage == 0)
				{
					SubChapter.FirstPage = Page.Number;
				}
				SubChapter.LastPage = Page.Number;
			}
		}
	}
}

FString UMarkdownLibrary::ResolveBookPath(const FString& InPath)
{
	if (InPath.IsEmpty())
	{
		return FString();
	}

	if (FPaths::IsRelative(InPath))
	{
		return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), InPath));
	}

	return FPaths::ConvertRelativePathToFull(InPath);
}

bool UMarkdownLibrary::LoadMarkdownFile(const FString& InPath, FString& OutMarkdown)
{
	const FString FullPath = ResolveBookPath(InPath);
	return !FullPath.IsEmpty() && FFileHelper::LoadFileToString(OutMarkdown, *FullPath);
}

bool UMarkdownLibrary::LoadMarkdownDocument(const FString& InPath, FMarkdownDocument& OutDocument)
{
	FString Markdown;
	if (!LoadMarkdownFile(InPath, Markdown))
	{
		return false;
	}

	OutDocument = FMarkdownDocument();
	OutDocument.SourcePath = ResolveBookPath(InPath);
	OutDocument.Blocks = ParseMarkdown(Markdown);

	for (const FMarkdownBlock& Block : OutDocument.Blocks)
	{
		if (Block.Type == EMarkdownBlockType::Heading && Block.Level == 1)
		{
			OutDocument.Title = Block.Text;
			break;
		}
	}

	if (OutDocument.Title.IsEmpty())
	{
		OutDocument.Title = FPaths::GetBaseFilename(OutDocument.SourcePath);
	}

	BuildStructure(OutDocument);

	return true;
}

TArray<FMarkdownBlock> UMarkdownLibrary::ParseMarkdown(const FString& Markdown)
{
	TArray<FMarkdownBlock> Blocks;

	FString Source = Markdown;
	if (Source.StartsWith(TEXT("\xFEFF")))
	{
		Source.RightChopInline(1);
	}

	TArray<FString> Lines;
	Source.ParseIntoArrayLines(Lines, false);

	TArray<FString> Paragraph;
	int32 ParagraphLine = 0;

	auto FlushParagraph = [&Blocks, &Paragraph, &ParagraphLine]()
	{
		if (Paragraph.Num() == 0)
		{
			return;
		}

		const FString Joined = FString::Join(Paragraph, TEXT(" "));
		Paragraph.Reset();
		if (!Joined.TrimStartAndEnd().IsEmpty())
		{
			Blocks.Add(MakeBlock(EMarkdownBlockType::Paragraph, Joined, ParagraphLine));
		}
	};

	for (int32 LineIndex = 0; LineIndex < Lines.Num();)
	{
		const FString Line = Lines[LineIndex].TrimStartAndEnd();
		const int32 LineNumber = LineIndex + 1;

		if (Line.IsEmpty())
		{
			FlushParagraph();
			++LineIndex;
			continue;
		}

		if (Line.StartsWith(TEXT("```")))
		{
			FlushParagraph();
			const int32 StartLine = LineNumber;
			TArray<FString> Code;
			++LineIndex;
			while (LineIndex < Lines.Num() && !Lines[LineIndex].TrimStartAndEnd().StartsWith(TEXT("```")))
			{
				Code.Add(Lines[LineIndex]);
				++LineIndex;
			}
			if (LineIndex < Lines.Num())
			{
				++LineIndex;
			}
			Blocks.Add(MakeBlock(EMarkdownBlockType::Code, FString::Join(Code, TEXT("\n")), StartLine));
			continue;
		}

		if (IsRule(Line))
		{
			FlushParagraph();
			Blocks.Add(MakeBlock(EMarkdownBlockType::Rule, FString(), LineNumber));
			++LineIndex;
			continue;
		}

		int32 HeadingLevel = 0;
		FString HeadingText;
		if (MatchHeading(Line, HeadingLevel, HeadingText))
		{
			FlushParagraph();
			Blocks.Add(IsPageMarker(HeadingText)
				? MakeBlock(EMarkdownBlockType::PageBreak, HeadingText, LineNumber)
				: MakeBlock(EMarkdownBlockType::Heading, HeadingText, LineNumber, HeadingLevel));
			++LineIndex;
			continue;
		}

		if (Line.StartsWith(TEXT(">"), ESearchCase::CaseSensitive))
		{
			FlushParagraph();
			const int32 StartLine = LineNumber;
			TArray<FString> Quote;
			while (LineIndex < Lines.Num())
			{
				const FString QuoteLine = Lines[LineIndex].TrimStartAndEnd();
				if (!QuoteLine.StartsWith(TEXT(">"), ESearchCase::CaseSensitive))
				{
					break;
				}
				Quote.Add(QuoteLine.Mid(1).TrimStartAndEnd());
				++LineIndex;
			}
			Blocks.Add(MakeBlock(EMarkdownBlockType::Quote, FString::Join(Quote, TEXT(" ")), StartLine));
			continue;
		}

		if (StartsWithMusicNote(Line))
		{
			FlushParagraph();
			Blocks.Add(MakeBlock(EMarkdownBlockType::MusicLink, Line, LineNumber));
			++LineIndex;
			continue;
		}

		FString ListText;
		if (MatchBullet(Line, ListText))
		{
			FlushParagraph();
			Blocks.Add(MakeBlock(EMarkdownBlockType::Bullet, ListText, LineNumber));
			++LineIndex;
			continue;
		}

		if (MatchNumbered(Line, ListText))
		{
			FlushParagraph();
			Blocks.Add(MakeBlock(EMarkdownBlockType::Numbered, ListText, LineNumber));
			++LineIndex;
			continue;
		}

		if (Paragraph.Num() == 0)
		{
			ParagraphLine = LineNumber;
		}
		Paragraph.Add(Line);
		++LineIndex;
	}

	FlushParagraph();
	return Blocks;
}

TArray<FString> UMarkdownLibrary::FindBookFiles()
{
	TArray<FString> Result;

	const FString BookDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Book"));
	if (!IFileManager::Get().DirectoryExists(*BookDir))
	{
		return Result;
	}

	TArray<FString> Found;
	IFileManager::Get().FindFilesRecursive(Found, *BookDir, TEXT("*.md"), true, false);

	for (const FString& File : Found)
	{
		FString Relative = FPaths::ConvertRelativePathToFull(File);
		FPaths::MakePathRelativeTo(Relative, *FPaths::ProjectDir());
		Result.Add(Relative);
	}

	Result.Sort();
	return Result;
}
