#pragma once

#include "CoreMinimal.h"
#include "BookMediaTypes.generated.h"

/** One narration recording for one subchapter. */
USTRUCT(BlueprintType)
struct FNarrationClip
{
	GENERATED_BODY()

	/** Subchapter title as written in the markdown; used to bind clips to the document. */
	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	FString SubChapter;

	/** Resolved when the manifest is bound to a document. */
	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	int32 SubChapterIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	FString Url;

	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	float DurationSec = 0.0f;
};

USTRUCT(BlueprintType)
struct FNarratorInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	FString Language;

	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	TArray<FNarrationClip> Clips;
};

/** Sidecar `<book>.media.json` next to the markdown file. */
USTRUCT(BlueprintType)
struct FBookMediaManifest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	FString BookId;

	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	FString Title;

	/** Opened by the Link mode. */
	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	FString PdfUrl;

	/** Base URL for streamed narration, e.g. https://media.example.com/lifedemandeddeath */
	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	FString StreamBaseUrl;

	/** Extension used when clip URLs are derived from the subchapter slug. */
	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	FString AudioExtension;

	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	FString DefaultNarrator;

	UPROPERTY(BlueprintReadOnly, Category = "Book|Media")
	TArray<FNarratorInfo> Narrators;
};
