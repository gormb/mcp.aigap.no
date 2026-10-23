#include "BookReaderActor.h"

#include "BookAudioDirectorComponent.h"
#include "BookMediaLibrary.h"
#include "BookSettings.h"
#include "BookStateSubsystem.h"
#include "Components/SceneComponent.h"
#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"
#include "MarkdownLibrary.h"
#include "UI/SBookReader.h"

DEFINE_LOG_CATEGORY_STATIC(LogBookReader, Log, All);

ABookReaderActor::ABookReaderActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	AudioDirector = CreateDefaultSubobject<UBookAudioDirectorComponent>(TEXT("BookAudioDirector"));
}

void ABookReaderActor::BeginPlay()
{
	Super::BeginPlay();

	if (UBookStateSubsystem* State = GetBookState())
	{
		PositionHandle = State->OnPositionChanged.AddUObject(this, &ABookReaderActor::HandleStatePositionChanged);
		ModeHandle = State->OnModeChanged.AddUObject(this, &ABookReaderActor::HandleStateModeChanged);
	}

	if (bOpenOnBeginPlay)
	{
		OpenBook();
	}
}

void ABookReaderActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UBookStateSubsystem* State = GetBookState())
	{
		State->OnPositionChanged.Remove(PositionHandle);
		State->OnModeChanged.Remove(ModeHandle);
	}

	CloseBook();
	Super::EndPlay(EndPlayReason);
}

UBookStateSubsystem* ABookReaderActor::GetBookState() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UBookStateSubsystem>() : nullptr;
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

	FBookMediaManifest Manifest;
	if (!UBookMediaLibrary::LoadManifest(RequestedPath, Manifest))
	{
		UE_LOG(LogBookReader, Log, TEXT("No media manifest for '%s'; using streaming defaults"), *RequestedPath);
	}

	if (!GEngine || !GEngine->GameViewport)
	{
		UE_LOG(LogBookReader, Warning, TEXT("No game viewport available to show the book in"));
		return false;
	}

	CloseBook();

	UBookStateSubsystem* State = GetBookState();
	if (State)
	{
		if (!bRestoreProgress)
		{
			State->ResetProgress();
		}
		State->OpenBook(RequestedPath, Document, Manifest);
	}

	ReaderWidget = SNew(SBookReader)
		.Document(Document)
		.OnClosed(FOnBookReaderClosed::CreateUObject(this, &ABookReaderActor::OnReaderClosed))
		.OnCycleMode(FOnBookReaderAction::CreateUObject(this, &ABookReaderActor::HandleCycleMode))
		.OnToggleNarration(FOnBookReaderAction::CreateUObject(this, &ABookReaderActor::HandleToggleNarration))
		.OnCycleNarrator(FOnBookReaderAction::CreateUObject(this, &ABookReaderActor::HandleCycleNarrator))
		.OnOpenLink(FOnBookReaderAction::CreateUObject(this, &ABookReaderActor::HandleOpenLink))
		.OnSubChapterChanged(FOnBookSubChapterChanged::CreateUObject(this, &ABookReaderActor::HandleSubChapterChanged))
		.OnModeSelected(FOnBookModeSelected::CreateUObject(this, &ABookReaderActor::HandleModeSelected));

	PushStateToReader();

	GEngine->GameViewport->AddViewportWidgetContent(ReaderWidget.ToSharedRef(), 10);

	if (ReaderWidget.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetAllUserFocus(ReaderWidget, EFocusCause::SetDirectly);
	}

	OpenBookPath = Document.SourcePath;
	UE_LOG(LogBookReader, Log, TEXT("Opened '%s' (%d blocks, %d subchapters, %d pages, title '%s')"),
		*OpenBookPath, Document.Blocks.Num(), Document.SubChapters.Num(), Document.Pages.Num(), *Document.Title);

	AudioDirector->RefreshForMode();
	return true;
}

void ABookReaderActor::CloseBook()
{
	if (ReaderWidget.IsValid())
	{
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(ReaderWidget.ToSharedRef());
		}
		ReaderWidget.Reset();
	}

	if (UBookStateSubsystem* State = GetBookState())
	{
		State->CloseBook();
	}

	OpenBookPath.Reset();
}

bool ABookReaderActor::IsBookOpen() const
{
	return ReaderWidget.IsValid();
}

void ABookReaderActor::PushStateToReader()
{
	UBookStateSubsystem* State = GetBookState();
	if (!State || !ReaderWidget.IsValid())
	{
		return;
	}

	ReaderWidget->SetMode(State->GetMode());
	ReaderWidget->SetNarratorName(State->GetNarratorDisplayName());
	ReaderWidget->SetPosition(State->GetPosition());

	TArray<FBookModeInfo> ModeInfos;
	State->GetModeInfos(ModeInfos);
	ReaderWidget->SetModeInfos(ModeInfos);
}

void ABookReaderActor::CycleMode()
{
	if (UBookStateSubsystem* State = GetBookState())
	{
		State->CycleMode();
	}
}

void ABookReaderActor::ToggleNarration()
{
	if (AudioDirector)
	{
		AudioDirector->ToggleNarration();
	}
}

void ABookReaderActor::OpenSourceLink()
{
	UBookStateSubsystem* State = GetBookState();
	FString Url = State ? State->GetManifest().PdfUrl : FString();
	if (Url.IsEmpty())
	{
		Url = UBookSettings::GetPdfUrl();
	}

	if (Url.IsEmpty())
	{
		UE_LOG(LogBookReader, Warning, TEXT("No source link configured (BookSettings.PdfUrl)"));
		return;
	}

	UE_LOG(LogBookReader, Log, TEXT("Opening source link %s"), *Url);
	FPlatformProcess::LaunchURL(*Url, nullptr, nullptr);
}

void ABookReaderActor::HandleCycleMode()
{
	CycleMode();
}

void ABookReaderActor::HandleToggleNarration()
{
	ToggleNarration();
}

void ABookReaderActor::HandleCycleNarrator()
{
	if (UBookStateSubsystem* State = GetBookState())
	{
		State->CycleNarrator();
		PushStateToReader();
	}
}

void ABookReaderActor::HandleOpenLink()
{
	OpenSourceLink();
}

void ABookReaderActor::HandleModeSelected(EExperienceMode NewMode)
{
	// Link is an action from the mode ladder, the reader state stays where it was.
	if (NewMode == EExperienceMode::Link)
	{
		OpenSourceLink();
		return;
	}

	if (UBookStateSubsystem* State = GetBookState())
	{
		State->SetModeByIndex(static_cast<int32>(NewMode));
	}
}

void ABookReaderActor::HandleSubChapterChanged(int32 SubChapterIndex)
{
	if (UBookStateSubsystem* State = GetBookState())
	{
		State->SetSubChapter(SubChapterIndex);
	}
}

void ABookReaderActor::HandleStatePositionChanged(const FBookPosition& NewPosition)
{
	if (ReaderWidget.IsValid())
	{
		ReaderWidget->SetPosition(NewPosition);
	}
}

void ABookReaderActor::HandleStateModeChanged(EExperienceMode NewMode)
{
	PushStateToReader();
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

	void BookModeCommand(const TArray<FString>& Args, UWorld* World)
	{
		ABookReaderActor* Reader = FindReaderInWorld(World);
		if (!Reader)
		{
			return;
		}

		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		UBookStateSubsystem* State = GameInstance ? GameInstance->GetSubsystem<UBookStateSubsystem>() : nullptr;
		if (!State)
		{
			return;
		}

		if (Args.Num() == 0)
		{
			UE_LOG(LogBookReader, Log, TEXT("Mode: %s"), *State->GetModeName());
			return;
		}

		State->SetModeByName(Args[0]);
	}

	void BookNarrationCommand(const TArray<FString>& Args, UWorld* World)
	{
		if (ABookReaderActor* Reader = FindReaderInWorld(World))
		{
			Reader->ToggleNarration();
		}
	}

	void BookNarratorCommand(const TArray<FString>& Args, UWorld* World)
	{
		UBookStateSubsystem* State = World && World->GetGameInstance()
			? World->GetGameInstance()->GetSubsystem<UBookStateSubsystem>()
			: nullptr;
		if (!State)
		{
			return;
		}

		if (Args.Num() == 0)
		{
			UE_LOG(LogBookReader, Log, TEXT("Narrator: %s"), *State->GetNarratorId());
			return;
		}

		State->SetNarrator(Args[0]);
	}

	void BookStepCommand(const TArray<FString>& Args, UWorld* World)
	{
		UBookStateSubsystem* State = World && World->GetGameInstance()
			? World->GetGameInstance()->GetSubsystem<UBookStateSubsystem>()
			: nullptr;
		if (!State || Args.Num() == 0)
		{
			return;
		}

		if (Args[0].Equals(TEXT("next"), ESearchCase::IgnoreCase))
		{
			State->NextSubChapter();
		}
		else if (Args[0].Equals(TEXT("prev"), ESearchCase::IgnoreCase))
		{
			State->PreviousSubChapter();
		}
	}

	void BookModesCommand(const TArray<FString>& Args, UWorld* World)
	{
		UBookStateSubsystem* State = World && World->GetGameInstance()
			? World->GetGameInstance()->GetSubsystem<UBookStateSubsystem>()
			: nullptr;
		if (!State)
		{
			return;
		}

		TArray<FBookModeInfo> Infos;
		State->GetModeInfos(Infos);
		UE_LOG(LogBookReader, Log, TEXT("Experience ladder (current %s):"), *State->GetModeName());
		for (int32 Index = 0; Index < Infos.Num(); ++Index)
		{
			const FBookModeInfo& Info = Infos[Index];
			const FString Status = Info.bAvailable ? TEXT("available") : Info.UnavailableReason;
			UE_LOG(LogBookReader, Log, TEXT("  M%d  %s  -  %s"), Index, *Info.Label, *Status);
		}
	}

	FAutoConsoleCommandWithWorldAndArgs GBookModesCommand(
		TEXT("Book.Modes"),
		TEXT("Lists the experience ladder with availability."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&BookModesCommand));

	void BookStatusCommand(const TArray<FString>& Args, UWorld* World)	{
		UBookStateSubsystem* State = World && World->GetGameInstance()
			? World->GetGameInstance()->GetSubsystem<UBookStateSubsystem>()
			: nullptr;
		if (!State)
		{
			return;
		}

		const FBookSubChapter* SubChapter = State->GetCurrentSubChapter();
		UE_LOG(LogBookReader, Log, TEXT("Book '%s' · mode %s · subchapter %d (%s) · page %d · narrator %s · narration %s"),
			*State->GetBookPath(),
			*State->GetModeName(),
			State->GetPosition().SubChapterIndex,
			SubChapter ? *SubChapter->Title : TEXT("none"),
			State->GetPosition().PageNumber,
			*State->GetNarratorId(),
			State->HasNarrationForCurrentSubChapter() ? TEXT("available") : TEXT("missing"));
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

	FAutoConsoleCommandWithWorldAndArgs GBookModeCommand(
		TEXT("Book.Mode"),
		TEXT("Gets or sets the experience mode: link, codex, score, soundbook, studio, vr, speculative."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&BookModeCommand));

	FAutoConsoleCommandWithWorldAndArgs GBookNarrationCommand(
		TEXT("Book.Narration"),
		TEXT("Toggles narration playback."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&BookNarrationCommand));

	FAutoConsoleCommandWithWorldAndArgs GBookNarratorCommand(
		TEXT("Book.Narrator"),
		TEXT("Gets or sets the narrator id, e.g. Book.Narrator guest-lotte."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&BookNarratorCommand));

	FAutoConsoleCommandWithWorldAndArgs GBookStepCommand(
		TEXT("Book.Step"),
		TEXT("Moves the shared cursor: Book.Step next | Book.Step prev."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&BookStepCommand));

	FAutoConsoleCommandWithWorldAndArgs GBookStatusCommand(
		TEXT("Book.Status"),
		TEXT("Logs the current book, mode, subchapter, page and narration availability."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&BookStatusCommand));
}
