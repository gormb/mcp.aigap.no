#pragma once

#include "CoreMinimal.h"
#include "BookTypes.h"
#include "MarkdownTypes.generated.h"

/** Structural element produced by the markdown parser. */
UENUM(BlueprintType)
enum class EMarkdownBlockType : uint8
{
	Paragraph,
	Heading,
	PageBreak,
	Bullet,
	Numbered,
	Quote,
	Code,
	Rule,
	MusicLink
};

USTRUCT(BlueprintType)
struct FMarkdownBlock
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Markdown")
	EMarkdownBlockType Type = EMarkdownBlockType::Paragraph;

	/** Text with inline markup (emphasis, links) resolved for display. */
	UPROPERTY(BlueprintReadOnly, Category = "Markdown")
	FString Text;

	/** Heading level (1-6) for Heading blocks, 0 otherwise. */
	UPROPERTY(BlueprintReadOnly, Category = "Markdown")
	int32 Level = 0;

	/** First link target found in the block, if any. */
	UPROPERTY(BlueprintReadOnly, Category = "Markdown")
	FString Url;

	/** 1-based source line the block starts on. */
	UPROPERTY(BlueprintReadOnly, Category = "Markdown")
	int32 Line = 0;
};

USTRUCT(BlueprintType)
struct FMarkdownDocument
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Markdown")
	FString SourcePath;

	UPROPERTY(BlueprintReadOnly, Category = "Markdown")
	FString Title;

	UPROPERTY(BlueprintReadOnly, Category = "Markdown")
	TArray<FMarkdownBlock> Blocks;

	/** Narration units derived from `##` / `###` headings. */
	UPROPERTY(BlueprintReadOnly, Category = "Markdown")
	TArray<FBookSubChapter> SubChapters;

	/** Page markers (`#### p. N`) with the blocks they cover. */
	UPROPERTY(BlueprintReadOnly, Category = "Markdown")
	TArray<FPageData> Pages;
};
