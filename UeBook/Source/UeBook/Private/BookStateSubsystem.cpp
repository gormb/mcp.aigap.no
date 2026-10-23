#include "BookStateSubsystem.h"

#include "BookMediaLibrary.h"
#include "BookSaveGame.h"
#include "BookSettings.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "UObject/Class.h"

DEFINE_LOG_CATEGORY_STATIC(LogBookState, Log, All);

void UBookStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UBookSettings* Settings = GetDefault<UBookSettings>();
	Mode = Settings->DefaultMode;
	NarratorId = Settings->DefaultNarratorId;
}

void UBookStateSubsystem::Deinitialize()
{
	SaveProgress();
	Super::Deinitialize();
}

void UBookStateSubsystem::OpenBook(const FString& InBookPath, const FMarkdownDocument& InDocument, const FBookMediaManifest& InManifest)
{
	BookPath = InBookPath;
	Document = InDocument;
	Manifest = InManifest;
	BookId = Manifest.BookId.IsEmpty() ? FPaths::GetBaseFilename(BookPath) : Manifest.BookId;
	bBookOpen = true;

	UBookMediaLibrary::BindClipsToDocument(Manifest, Document.SubChapters);

	const UBookSettings* Settings = GetDefault<UBookSettings>();
	if (NarratorId.IsEmpty())
	{
		NarratorId = Manifest.DefaultNarrator.IsEmpty() ? Settings->DefaultNarratorId : Manifest.DefaultNarrator;
	}

	Position = FBookPosition();
	Mode = Settings->DefaultMode;
	LoadProgress();

	if (Position.PageNumber == 0 && Document.SubChapters.Num() > 0)
	{
		Position.PageNumber = Document.SubChapters[Position.SubChapterIndex].FirstPage;
	}

	BroadcastMode();
	OnPositionChanged.Broadcast(Position);
}

void UBookStateSubsystem::CloseBook()
{
	if (!bBookOpen)
	{
		return;
	}

	SaveProgress();

	bBookOpen = false;
	Document = FMarkdownDocument();
	Manifest = FBookMediaManifest();
	BookPath.Reset();
	BookId.Reset();
	Position = FBookPosition();
}

void UBookStateSubsystem::SetSubChapter(int32 SubChapterIndex)
{
	if (!Document.SubChapters.IsValidIndex(SubChapterIndex))
	{
		return;
	}

	FBookPosition NewPosition = Position;
	NewPosition.SubChapterIndex = SubChapterIndex;
	NewPosition.PageNumber = Document.SubChapters[SubChapterIndex].FirstPage;
	ApplyPosition(NewPosition);
}

void UBookStateSubsystem::NextSubChapter()
{
	SetSubChapter(Position.SubChapterIndex + 1);
}

void UBookStateSubsystem::PreviousSubChapter()
{
	SetSubChapter(Position.SubChapterIndex - 1);
}

void UBookStateSubsystem::ApplyPosition(const FBookPosition& NewPosition)
{
	if (Position == NewPosition)
	{
		return;
	}

	Position = NewPosition;
	OnPositionChanged.Broadcast(Position);
}

void UBookStateSubsystem::SetMode(EExperienceMode NewMode)
{
	if (NewMode == EExperienceMode::Link)
	{
		// Link opens the manuscript in the browser; it is an action, not a reading state.
		UE_LOG(LogBookState, Warning, TEXT("Link mode is an action; keeping mode %s"), *GetModeName());
		return;
	}

	EExperienceMode EffectiveMode = NewMode;

	if (NewMode == EExperienceMode::VRWorld && (!GEngine || !GEngine->XRSystem.IsValid()))
	{
		UE_LOG(LogBookState, Warning, TEXT("No XR system present; VR World falls back to Studio"));
		EffectiveMode = EExperienceMode::Studio;
	}

	if (Mode == EffectiveMode)
	{
		return;
	}

	Mode = EffectiveMode;
	BroadcastMode();
}

void UBookStateSubsystem::CycleMode()
{
	// Skip modes that cannot run right now instead of cycling through dead ends.
	const int32 Count = GetModeCount();
	for (int32 Step = 1; Step <= Count; ++Step)
	{
		const int32 Candidate = (static_cast<int32>(Mode) + Step) % Count;
		FString Reason;
		if (static_cast<EExperienceMode>(Candidate) != EExperienceMode::Link
			&& IsModeAvailable(static_cast<EExperienceMode>(Candidate), Reason))
		{
			SetMode(static_cast<EExperienceMode>(Candidate));
			return;
		}
	}
}

int32 UBookStateSubsystem::GetModeCount()
{
	return static_cast<int32>(EExperienceMode::Speculative) + 1;
}

void UBookStateSubsystem::SetModeByIndex(int32 Index)
{
	if (Index < 0 || Index >= GetModeCount())
	{
		return;
	}

	const EExperienceMode Requested = static_cast<EExperienceMode>(Index);
	FString Reason;
	if (!IsModeAvailable(Requested, Reason))
	{
		UE_LOG(LogBookState, Warning, TEXT("Mode %d unavailable: %s"), Index, *Reason);
		return;
	}

	SetMode(Requested);
}

bool UBookStateSubsystem::IsModeAvailable(EExperienceMode InMode, FString& OutReason) const
{
	const UBookSettings* Settings = GetDefault<UBookSettings>();

	switch (InMode)
	{
	case EExperienceMode::Link:
	{
		const FString Url = Manifest.PdfUrl.IsEmpty() ? Settings->PdfUrl : Manifest.PdfUrl;
		if (Url.IsEmpty())
		{
			OutReason = TEXT("No source link configured");
			return false;
		}
		return true;
	}

	case EExperienceMode::Codex:
		return true;

	case EExperienceMode::Score:
		if (Settings->MusicStreamUrl.IsEmpty())
		{
			OutReason = TEXT("No music stream configured");
			return false;
		}
		return true;

	case EExperienceMode::SoundBook:
	case EExperienceMode::Studio:
		if (!bBookOpen || Document.SubChapters.Num() == 0)
		{
			OutReason = TEXT("Open a book first");
			return false;
		}
		if (Manifest.Narrators.Num() == 0)
		{
			OutReason = TEXT("No narrator in the media manifest");
			return false;
		}
		if (Manifest.StreamBaseUrl.IsEmpty())
		{
			OutReason = TEXT("No narration stream base URL");
			return false;
		}
		return true;

	case EExperienceMode::VRWorld:
		if (!GEngine || !GEngine->XRSystem.IsValid())
		{
			OutReason = TEXT("No XR headset detected");
			return false;
		}
		return true;

	case EExperienceMode::Speculative:
		OutReason = TEXT("Research playground - not available yet");
		return false;
	}

	return true;
}

void UBookStateSubsystem::GetModeInfos(TArray<FBookModeInfo>& OutInfos) const
{
	OutInfos.Reset();

	for (int32 Index = 0; Index < GetModeCount(); ++Index)
	{
		const EExperienceMode Current = static_cast<EExperienceMode>(Index);

		FBookModeInfo Info;
		Info.Mode = Current;
		Info.bAvailable = IsModeAvailable(Current, Info.UnavailableReason);

		switch (Current)
		{
		case EExperienceMode::Link:
			Info.Label = TEXT("Link");
			Info.Description = TEXT("Open the source manuscript in your browser");
			break;
		case EExperienceMode::Codex:
			Info.Label = TEXT("Codex");
			Info.Description = TEXT("Typographic reading, no audio");
			break;
		case EExperienceMode::Score:
			Info.Label = TEXT("Score");
			Info.Description = TEXT("Reading with the soundtrack underneath");
			break;
		case EExperienceMode::SoundBook:
			Info.Label = TEXT("Sound Book");
			Info.Description = TEXT("Recorded narration, read-along scrolling");
			break;
		case EExperienceMode::Studio:
			Info.Label = TEXT("Studio");
			Info.Description = TEXT("Spatial narration and ambience");
			break;
		case EExperienceMode::VRWorld:
			Info.Label = TEXT("VR World");
			Info.Description = TEXT("Step inside the book in VR");
			break;
		case EExperienceMode::Speculative:
			Info.Label = TEXT("Speculative");
			Info.Description = TEXT("Generative worlds and reader experiments");
			break;
		}

		OutInfos.Add(Info);
	}
}

void UBookStateSubsystem::SetModeByName(const FString& ModeName)
{
	const FString Name = ModeName.ToLower();

	if (Name == TEXT("link")) { SetMode(EExperienceMode::Link); }
	else if (Name == TEXT("codex")) { SetMode(EExperienceMode::Codex); }
	else if (Name == TEXT("score")) { SetMode(EExperienceMode::Score); }
	else if (Name == TEXT("soundbook") || Name == TEXT("sound-book") || Name == TEXT("sound_book")) { SetMode(EExperienceMode::SoundBook); }
	else if (Name == TEXT("studio")) { SetMode(EExperienceMode::Studio); }
	else if (Name == TEXT("vr") || Name == TEXT("vrworld")) { SetMode(EExperienceMode::VRWorld); }
	else if (Name == TEXT("speculative")) { SetMode(EExperienceMode::Speculative); }
	else
	{
		UE_LOG(LogBookState, Warning, TEXT("Unknown mode '%s'"), *ModeName);
	}
}

FString UBookStateSubsystem::GetModeName() const
{
	return StaticEnum<EExperienceMode>()->GetNameStringByValue(static_cast<int64>(Mode));
}

void UBookStateSubsystem::BroadcastMode()
{
	OnModeChanged.Broadcast(Mode);
}

FString UBookStateSubsystem::GetNarratorDisplayName() const
{
	FNarratorInfo Narrator;
	if (UBookMediaLibrary::FindNarrator(Manifest, NarratorId, Narrator))
	{
		return Narrator.DisplayName.IsEmpty() ? Narrator.Id : Narrator.DisplayName;
	}
	return NarratorId;
}

void UBookStateSubsystem::CycleNarrator()
{
	if (Manifest.Narrators.Num() < 2)
	{
		return;
	}

	int32 CurrentIndex = INDEX_NONE;
	for (int32 Index = 0; Index < Manifest.Narrators.Num(); ++Index)
	{
		if (Manifest.Narrators[Index].Id.Equals(NarratorId, ESearchCase::IgnoreCase))
		{
			CurrentIndex = Index;
			break;
		}
	}

	NarratorId = Manifest.Narrators[(CurrentIndex + 1) % Manifest.Narrators.Num()].Id;
	BroadcastMode();
}

void UBookStateSubsystem::SetNarrator(const FString& InNarratorId)
{
	FNarratorInfo Narrator;
	if (UBookMediaLibrary::FindNarrator(Manifest, InNarratorId, Narrator))
	{
		NarratorId = Narrator.Id;
		BroadcastMode();
	}
}

const FBookSubChapter* UBookStateSubsystem::GetCurrentSubChapter() const
{
	if (!Document.SubChapters.IsValidIndex(Position.SubChapterIndex))
	{
		return nullptr;
	}
	return &Document.SubChapters[Position.SubChapterIndex];
}

bool UBookStateSubsystem::HasNarrationForCurrentSubChapter() const
{
	const FBookSubChapter* SubChapter = GetCurrentSubChapter();
	if (!SubChapter)
	{
		return false;
	}

	const FString Url = UBookMediaLibrary::ResolveNarrationUrl(Manifest, NarratorId, Position.SubChapterIndex, *SubChapter);
	return !Url.IsEmpty();
}

void UBookStateSubsystem::SaveProgress()
{
	if (!bBookOpen)
	{
		return;
	}

	UBookSaveGame* Save = Cast<UBookSaveGame>(UGameplayStatics::CreateSaveGameObject(UBookSaveGame::StaticClass()));
	if (!Save)
	{
		return;
	}

	Save->BookPath = BookPath;
	Save->BookId = BookId;
	Save->SubChapterIndex = Position.SubChapterIndex;
	Save->PageNumber = Position.PageNumber;
	Save->Mode = Mode;
	Save->NarratorId = NarratorId;

	UGameplayStatics::SaveGameToSlot(Save, GetSaveSlotName(), 0);
}

void UBookStateSubsystem::LoadProgress()
{
	const UBookSaveGame* Save = Cast<UBookSaveGame>(UGameplayStatics::LoadGameFromSlot(GetSaveSlotName(), 0));
	if (!Save)
	{
		return;
	}

	const bool bSamePath = Save->BookPath.IsEmpty() || Save->BookPath.Equals(BookPath, ESearchCase::IgnoreCase);
	const bool bSameBook = Save->BookId.IsEmpty() || BookId.IsEmpty() || Save->BookId.Equals(BookId, ESearchCase::IgnoreCase);
	if (!bSamePath || !bSameBook)
	{
		return;
	}

	if (Document.SubChapters.IsValidIndex(Save->SubChapterIndex))
	{
		Position.SubChapterIndex = Save->SubChapterIndex;
	}
	Position.PageNumber = Save->PageNumber;
	Mode = Save->Mode;

	if (!Save->NarratorId.IsEmpty())
	{
		NarratorId = Save->NarratorId;
	}

	UE_LOG(LogBookState, Log, TEXT("Resumed '%s' at subchapter %d, page %d, mode %s"),
		*BookPath, Position.SubChapterIndex, Position.PageNumber, *GetModeName());
}

void UBookStateSubsystem::ResetProgress()
{
	UGameplayStatics::DeleteGameInSlot(GetSaveSlotName(), 0);

	Position = FBookPosition();
	Mode = GetDefault<UBookSettings>()->DefaultMode;
	NarratorId = GetDefault<UBookSettings>()->DefaultNarratorId;
}
