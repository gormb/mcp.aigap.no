#include "BookReaderActor.h"

#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"
#include "MarkdownLibrary.h"
#include "UI/SBookReader.h"

DEFINE_LOG_CATEGORY_STATIC(LogBookReader, Log, All);

ABookReaderActor::ABookReaderActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABookReaderActor::BeginPlay()
{
	Super::BeginPlay();

	if (bOpenOnBeginPlay)
	{
		OpenBook();
	}
}

void ABookReaderActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CloseBook();
	Super::EndPlay(EndPlayReason);
}

bool ABookReaderActor::OpenBook(const FString& InPath)
{
	const FString RequestedPath = InPath.IsEmpty() ? BookPath : InPath;

	FMarkdownDocument Document;
	if (!UMarkdownLibrary::LoadMarkdownDocument(RequestedPath, Document))
	{
		UE_LOG(LogBookReader, Error, TEXT("Could not read markdown file '%s'"),
			*UMarkdownLibrary::ResolveBookPath(RequestedPath));
		return false;
	}

	if (!GEngine || !GEngine->GameViewport)
	{
		UE_LOG(LogBookReader, Warning, TEXT("No game viewport available to show the book in"));
		return false;
	}

	CloseBook();

	ReaderWidget = SNew(SBookReader)
		.Document(Document)
		.OnClosed(FOnBookReaderClosed::CreateUObject(this, &ABookReaderActor::OnReaderClosed));

	GEngine->GameViewport->AddViewportWidgetContent(ReaderWidget.ToSharedRef(), 10);

	if (ReaderWidget.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetAllUserFocus(ReaderWidget, EFocusCause::SetDirectly);
	}

	OpenBookPath = Document.SourcePath;
	UE_LOG(LogBookReader, Log, TEXT("Opened '%s' (%d blocks, title '%s')"),
		*OpenBookPath, Document.Blocks.Num(), *Document.Title);

	if (FParse::Param(FCommandLine::Get(), TEXT("BookTestJump")))
	{
		TWeakPtr<SWidget> WeakReader = ReaderWidget;
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakReader](float)
		{
			if (TSharedPtr<SWidget> Widget = WeakReader.Pin())
			{
				StaticCastSharedPtr<SBookReader>(Widget)->JumpToChapter(6);
			}
			return false;
		}), 5.0f);
	}

	return true;
}

void ABookReaderActor::CloseBook()
{
	if (!ReaderWidget.IsValid())
	{
		return;
	}

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(ReaderWidget.ToSharedRef());
	}

	ReaderWidget.Reset();
	OpenBookPath.Reset();
}

bool ABookReaderActor::IsBookOpen() const
{
	return ReaderWidget.IsValid();
}

void ABookReaderActor::OnReaderClosed()
{
	// Removing the focused widget from inside its own key event is unsafe, so wait one frame.
	TWeakObjectPtr<ABookReaderActor> WeakThis(this);
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakThis](float)
	{
		if (ABookReaderActor* Reader = WeakThis.Get())
		{
			Reader->CloseBook();
		}
		return false;
	}));
}

namespace
{
	ABookReaderActor* FindReaderInWorld(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		for (TActorIterator<ABookReaderActor> It(World); It; ++It)
		{
			return *It;
		}

		return nullptr;
	}

	void BookOpenCommand(const TArray<FString>& Args, UWorld* World)
	{
		ABookReaderActor* Reader = FindReaderInWorld(World);
		if (!Reader)
		{
			UE_LOG(LogBookReader, Error, TEXT("Book.Open: no ABookReaderActor in the current level"));
			return;
		}

		if (!Reader->OpenBook(Args.Num() > 0 ? FString::Join(Args, TEXT(" ")) : FString()))
		{
			UE_LOG(LogBookReader, Error, TEXT("Book.Open: failed to open the requested file"));
		}
	}

	void BookCloseCommand(const TArray<FString>& Args, UWorld* World)
	{
		if (ABookReaderActor* Reader = FindReaderInWorld(World))
		{
			Reader->CloseBook();
		}
	}

	void BookListCommand(const TArray<FString>& Args, UWorld* World)
	{
		const TArray<FString> Files = UMarkdownLibrary::FindBookFiles();
		UE_LOG(LogBookReader, Log, TEXT("%d markdown file(s) in the Book folder:"), Files.Num());
		for (const FString& File : Files)
		{
			UE_LOG(LogBookReader, Log, TEXT("  %s"), *File);
		}
	}

	FAutoConsoleCommandWithWorldAndArgs GBookOpenCommand(
		TEXT("Book.Open"),
		TEXT("Opens a markdown file in the book reader, relative to the project folder. Usage: Book.Open Book/b_NO_FREE.md"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&BookOpenCommand));

	FAutoConsoleCommandWithWorldAndArgs GBookCloseCommand(
		TEXT("Book.Close"),
		TEXT("Closes the book reader overlay."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&BookCloseCommand));

	FAutoConsoleCommandWithWorldAndArgs GBookListCommand(
		TEXT("Book.List"),
		TEXT("Lists the markdown files available in the project's Book folder."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&BookListCommand));
}
