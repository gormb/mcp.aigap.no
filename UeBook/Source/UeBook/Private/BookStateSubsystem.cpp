#include "BookStateSubsystem.h"

#include "BookMediaLibrary.h"
#include "BookSaveGame.h"
#include "BookSettings.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "UObject/Class.h"

DEFINE_LOG_CATEGORY_STATIC(LogBookState, Log, All);

namespace
{
	/** Modes the player can cycle through with the M key. Link/VR/Speculative are explicit commands. */
	const TArray<EExperienceMode>& GetCycledModes()
	{
		static const TArray<EExperienceMode> Modes = {
			EExperienceMode::Codex,
			EExperienceMode::Score,
			EExperienceMode::SoundBook,
			EExperienceMode::Studio
		};
		return Modes;
	}
}

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
	const TArray<EExperienceMode>& Modes = GetCycledModes();
	const int32 CurrentIndex = Modes.IndexOfByKey(Mode);
	const int32 NextIndex = CurrentIndex == INDEX_NONE ? 0 : (CurrentIndex + 1) % Modes.Num();
	SetMode(Modes[NextIndex]);
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
