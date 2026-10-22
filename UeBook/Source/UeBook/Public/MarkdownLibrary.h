#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MarkdownTypes.h"
#include "MarkdownLibrary.generated.h"

/**
 * Reads markdown files from disk and parses them into display blocks.
 * Paths are absolute or relative to the project directory, so the same
 * value works in the editor and in a packaged build.
 */
UCLASS()
class UEBOOK_API UMarkdownLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Turns a relative book path into an absolute one rooted at the project directory. */
	UFUNCTION(BlueprintPure, Category = "Book|Markdown")
	static FString ResolveBookPath(const FString& InPath);

	/** Reads a markdown file as UTF-8 text. */
	UFUNCTION(BlueprintCallable, Category = "Book|Markdown")
	static bool LoadMarkdownFile(const FString& InPath, FString& OutMarkdown);

	/** Reads and parses a markdown file. */
	UFUNCTION(BlueprintCallable, Category = "Book|Markdown")
	static bool LoadMarkdownDocument(const FString& InPath, FMarkdownDocument& OutDocument);

	/** Parses markdown source into blocks. */
	UFUNCTION(BlueprintPure, Category = "Book|Markdown")
	static TArray<FMarkdownBlock> ParseMarkdown(const FString& Markdown);

	/** Markdown files found in the project's Book folder, relative to the project directory. */
	UFUNCTION(BlueprintCallable, Category = "Book|Markdown")
	static TArray<FString> FindBookFiles();
};
