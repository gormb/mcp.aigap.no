#pragma once

#include "CoreMinimal.h"
#include "BookTypes.h"
#include "GameFramework/Actor.h"
#include "BookReaderActor.generated.h"

class SBookReader;
class UBookAudioDirectorComponent;
class UBookStateSubsystem;
class USceneComponent;

/**
 * Places the markdown reader in the viewport and drives the shared book state:
 * experience mode, reading cursor, narration and music.
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

	/** Restore the last subchapter/page/mode for this book. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Book")
	bool bRestoreProgress = true;

	/** Opens the reader, optionally with another file than BookPath. */
	UFUNCTION(BlueprintCallable, Category = "Book")
	bool OpenBook(const FString& InPath = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Book")
	void CloseBook();

	UFUNCTION(BlueprintPure, Category = "Book")
	bool IsBookOpen() const;

	UFUNCTION(BlueprintPure, Category = "Book")
	FString GetOpenBookPath() const { return OpenBookPath; }

	UFUNCTION(BlueprintCallable, Category = "Book")
	void CycleMode();

	UFUNCTION(BlueprintCallable, Category = "Book")
	void ToggleNarration();

	/** Opens the manuscript (PDF/HTML) of the current book in the system browser. */
	UFUNCTION(BlueprintCallable, Category = "Book")
	void OpenSourceLink();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void OnReaderClosed();
	void HandleCycleMode();
	void HandleToggleNarration();
	void HandleCycleNarrator();
	void HandleOpenLink();
	void HandleSubChapterChanged(int32 SubChapterIndex);
	void HandleStatePositionChanged(const FBookPosition& NewPosition);
	void HandleStateModeChanged(EExperienceMode NewMode);
	void PushStateToReader();

	UBookStateSubsystem* GetBookState() const;

	UPROPERTY(VisibleAnywhere, Category = "Book")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Book")
	TObjectPtr<UBookAudioDirectorComponent> AudioDirector;

	TSharedPtr<SBookReader> ReaderWidget;
	FString OpenBookPath;
	FDelegateHandle PositionHandle;
	FDelegateHandle ModeHandle;
};
