#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BookReaderActor.generated.h"

/**
 * Places a scrollable markdown reader in the viewport.
 * Defaults to the shipped book, but any .md path can be passed to OpenBook().
 */
UCLASS()
class UEBOOK_API ABookReaderActor : public AActor
{
	GENERATED_BODY()

public:
	ABookReaderActor();

	/** Markdown file to show, absolute or relative to the project directory. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Book")
	FString BookPath = TEXT("Book/b_NO_FREE.md");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Book")
	bool bOpenOnBeginPlay = true;

	/** Opens the reader, optionally with another file than BookPath. */
	UFUNCTION(BlueprintCallable, Category = "Book")
	bool OpenBook(const FString& InPath = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Book")
	void CloseBook();

	UFUNCTION(BlueprintPure, Category = "Book")
	bool IsBookOpen() const;

	UFUNCTION(BlueprintPure, Category = "Book")
	FString GetOpenBookPath() const { return OpenBookPath; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void OnReaderClosed();

	TSharedPtr<SWidget> ReaderWidget;
	FString OpenBookPath;
};
