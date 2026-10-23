#pragma once

#include "CoreMinimal.h"
#include "BookMediaTypes.h"
#include "BookTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BookMediaLibrary.generated.h"

/** Loads the media sidecar next to a book and resolves narration URLs. */
UCLASS()
class UEBOOK_API UBookMediaLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** `<book dir>/<book name>.media.json` for a given markdown path. */
	UFUNCTION(BlueprintPure, Category = "Book|Media")
	static FString GetManifestPath(const FString& BookPath);

	/**
	 * Loads the sidecar manifest. When it is missing, a default manifest is built from the
	 * project settings so streaming URLs can still be derived from subchapter slugs.
	 */
	UFUNCTION(BlueprintCallable, Category = "Book|Media")
	static bool LoadManifest(const FString& BookPath, FBookMediaManifest& OutManifest);

	UFUNCTION(BlueprintPure, Category = "Book|Media")
	static bool FindNarrator(const FBookMediaManifest& Manifest, const FString& NarratorId, FNarratorInfo& OutNarrator);

	/** Explicit clip URL when the manifest lists one, otherwise `<base>/<narrator>/<slug>.<ext>`. */
	UFUNCTION(BlueprintPure, Category = "Book|Media")
	static FString ResolveNarrationUrl(
		const FBookMediaManifest& Manifest,
		const FString& NarratorId,
		int32 SubChapterIndex,
		const FBookSubChapter& SubChapter);

	/** Binds `SubChapter` clip names to indices in the parsed document. */
	static void BindClipsToDocument(FBookMediaManifest& Manifest, const TArray<FBookSubChapter>& SubChapters);
};
