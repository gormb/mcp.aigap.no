#pragma once

#include "CoreMinimal.h"
#include "BookTypes.generated.h"

/** Ways of experiencing a book. The parser and cursor are shared, only the presentation changes. */
UENUM(BlueprintType)
enum class EExperienceMode : uint8
{
	/** Opens the original manuscript (PDF/HTML) in the OS browser. */
	Link UMETA(DisplayName = "Link out"),
	/** Typographic reading, no audio. */
	Codex UMETA(DisplayName = "Codex"),
	/** Reading with the chapter soundtrack and ambience. */
	Score UMETA(DisplayName = "Score"),
	/** Recorded narration with read-along scrolling. */
	SoundBook UMETA(DisplayName = "Sound Book"),
	/** Spatial staging of the narration and ambience. */
	Studio UMETA(DisplayName = "Studio"),
	/** Quest/OpenXR world; falls back to Studio when no headset is present. */
	VRWorld UMETA(DisplayName = "VR World"),
	/** R&D playground (generative worlds, ghost reader). */
	Speculative UMETA(DisplayName = "Speculative")
};

/** One page marker (`#### p. N`) and the blocks it covers. */
USTRUCT(BlueprintType)
struct FPageData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Book")
	int32 Number = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Book")
	int32 FirstBlock = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Book")
	int32 LastBlock = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Book")
	int32 SubChapterIndex = INDEX_NONE;
};

/** A narration unit: one `##` or `###` heading and everything under it. */
USTRUCT(BlueprintType)
struct FBookSubChapter
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Book")
	FString Title;

	UPROPERTY(BlueprintReadOnly, Category = "Book")
	int32 HeadingLevel = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Book")
	int32 FirstBlock = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Book")
	int32 LastBlock = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Book")
	int32 FirstPage = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Book")
	int32 LastPage = 0;

	/** Stable, file-name friendly id used to derive narration URLs, e.g. "02-nullsum". */
	UPROPERTY(BlueprintReadOnly, Category = "Book")
	FString Slug;
};

/** The single cursor shared by every experience mode. */
USTRUCT(BlueprintType)
struct FBookPosition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Book")
	int32 SubChapterIndex = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Book")
	int32 PageNumber = 0;

	bool operator==(const FBookPosition& Other) const
	{
		return SubChapterIndex == Other.SubChapterIndex && PageNumber == Other.PageNumber;
	}

	bool operator!=(const FBookPosition& Other) const { return !(*this == Other); }
};
