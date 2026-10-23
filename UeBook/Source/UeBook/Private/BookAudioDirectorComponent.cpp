#include "BookAudioDirectorComponent.h"

#include "BookMediaLibrary.h"
#include "BookSettings.h"
#include "BookStateSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogBookAudio, Log, All);

UBookAudioDirectorComponent::UBookAudioDirectorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UBookAudioDirectorComponent::BeginPlay()
{
	Super::BeginPlay();

	NarrationPlayer = NewObject<UMediaPlayer>(this, TEXT("BookNarrationPlayer"));
	MusicPlayer = NewObject<UMediaPlayer>(this, TEXT("BookMusicPlayer"));

	if (NarrationPlayer)
	{
		NarrationPlayer->OnEndReached.AddDynamic(this, &UBookAudioDirectorComponent::HandleNarrationFinished);
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	NarrationSound = NewObject<UMediaSoundComponent>(Owner, TEXT("BookNarrationSound"));
	MusicSound = NewObject<UMediaSoundComponent>(Owner, TEXT("BookMusicSound"));

	auto SetupSound = [Owner](UMediaSoundComponent* Sound, UMediaPlayer* Player)
	{
		if (!Sound)
		{
			return;
		}

		Sound->RegisterComponent();
		if (Owner->GetRootComponent())
		{
			Sound->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		}
		Sound->SetMediaPlayer(Player);
		Sound->SetVolumeMultiplier(0.0f);
		Sound->bAllowSpatialization = false;
	};

	SetupSound(NarrationSound, NarrationPlayer);
	SetupSound(MusicSound, MusicPlayer);

	if (UBookStateSubsystem* State = GetBookState())
	{
		State->OnPositionChanged.AddUObject(this, &UBookAudioDirectorComponent::HandlePositionChanged);
		State->OnModeChanged.AddUObject(this, &UBookAudioDirectorComponent::HandleModeChanged);
		bHandlesBound = true;
	}

	RefreshForMode();
}

void UBookAudioDirectorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bHandlesBound)
	{
		if (UBookStateSubsystem* State = GetBookState())
		{
			State->OnPositionChanged.RemoveAll(this);
			State->OnModeChanged.RemoveAll(this);
		}
		bHandlesBound = false;
	}

	if (NarrationPlayer)
	{
		NarrationPlayer->OnEndReached.RemoveAll(this);
		NarrationPlayer->Close();
	}
	if (MusicPlayer)
	{
		MusicPlayer->Close();
	}

	if (NarrationSound)
	{
		NarrationSound->Stop();
	}
	if (MusicSound)
	{
		MusicSound->Stop();
	}

	Super::EndPlay(EndPlayReason);
}

void UBookAudioDirectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const float NarrationTarget = (bNarrationRequested && IsNarrationPlaying()) ? 1.0f : 0.0f;
	NarrationVolume = FMath::FInterpTo(NarrationVolume, NarrationTarget, DeltaTime, 8.0f);
	if (NarrationSound)
	{
		NarrationSound->SetVolumeMultiplier(NarrationVolume);
	}

	// Music sits under the narration instead of fighting it.
	const float Duck = NarrationVolume > 0.01f ? 0.25f : 1.0f;
	const float MusicTarget = (bMusicRequested ? 1.0f : 0.0f) * Duck;
	MusicVolume = FMath::FInterpTo(MusicVolume, MusicTarget, DeltaTime, 3.0f);
	if (MusicSound)
	{
		MusicSound->SetVolumeMultiplier(MusicVolume);
	}
}

UBookStateSubsystem* UBookAudioDirectorComponent::GetBookState() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const UGameInstance* GameInstance = World->GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UBookStateSubsystem>() : nullptr;
}

void UBookAudioDirectorComponent::RefreshForMode()
{
	const UBookStateSubsystem* State = GetBookState();
	const EExperienceMode Mode = State ? State->GetMode() : EExperienceMode::Codex;

	const bool bWantsNarration = Mode == EExperienceMode::SoundBook || Mode == EExperienceMode::Studio;
	const bool bWantsMusic = Mode == EExperienceMode::Score || Mode == EExperienceMode::SoundBook || Mode == EExperienceMode::Studio;

	if (NarrationSound)
	{
		NarrationSound->bAllowSpatialization = Mode == EExperienceMode::Studio;
	}

	if (bWantsMusic)
	{
		StartMusic();
	}
	else
	{
		StopMusic();
	}

	if (bWantsNarration)
	{
		PlayNarration();
	}
	else
	{
		StopNarration();
	}
}

void UBookAudioDirectorComponent::PlayNarration()
{
	UBookStateSubsystem* State = GetBookState();
	if (!State || !State->IsBookOpen() || !NarrationPlayer)
	{
		return;
	}

	const FBookSubChapter* SubChapter = State->GetCurrentSubChapter();
	if (!SubChapter)
	{
		return;
	}

	const FString Url = UBookMediaLibrary::ResolveNarrationUrl(
		State->GetManifest(), State->GetNarratorId(), State->GetPosition().SubChapterIndex, *SubChapter);

	if (Url.IsEmpty())
	{
		bNarrationRequested = false;
		UE_LOG(LogBookAudio, Warning, TEXT("No narration source for '%s'"), *SubChapter->Title);
		return;
	}

	bNarrationRequested = true;

	if (Url != CurrentNarrationUrl || !NarrationPlayer->IsPlaying())
	{
		CurrentNarrationUrl = Url;

		if (!NarrationPlayer->OpenUrl(Url))
		{
			UE_LOG(LogBookAudio, Error, TEXT("Could not open narration stream '%s'"), *Url);
			bNarrationRequested = false;
			return;
		}

		NarrationPlayer->Play();
		UE_LOG(LogBookAudio, Log, TEXT("Narration: %s"), *Url);
	}
}

void UBookAudioDirectorComponent::StopNarration()
{
	bNarrationRequested = false;
	CurrentNarrationUrl.Reset();

	if (NarrationPlayer)
	{
		NarrationPlayer->Close();
	}
}

void UBookAudioDirectorComponent::ToggleNarration()
{
	if (bNarrationRequested)
	{
		StopNarration();
	}
	else
	{
		PlayNarration();
	}
}

bool UBookAudioDirectorComponent::IsNarrationPlaying() const
{
	return NarrationPlayer && NarrationPlayer->IsPlaying();
}

void UBookAudioDirectorComponent::StartMusic()
{
	const UBookSettings* Settings = GetDefault<UBookSettings>();
	const FString Url = Settings->MusicStreamUrl;

	if (Url.IsEmpty() || !MusicPlayer)
	{
		return;
	}

	if (CurrentMusicUrl == Url && MusicPlayer->IsPlaying())
	{
		bMusicRequested = true;
		return;
	}

	CurrentMusicUrl = Url;
	if (MusicPlayer->OpenUrl(Url))
	{
		MusicPlayer->Play();
		bMusicRequested = true;
		UE_LOG(LogBookAudio, Log, TEXT("Music: %s"), *Url);
	}
	else
	{
		UE_LOG(LogBookAudio, Warning, TEXT("Could not open music stream '%s'"), *Url);
	}
}

void UBookAudioDirectorComponent::StopMusic()
{
	bMusicRequested = false;
	if (MusicPlayer)
	{
		MusicPlayer->Close();
	}
	CurrentMusicUrl.Reset();
}

void UBookAudioDirectorComponent::HandleNarrationFinished()
{
	UBookStateSubsystem* State = GetBookState();
	if (!bNarrationRequested || !State || !State->IsBookOpen())
	{
		return;
	}

	const int32 NextIndex = State->GetPosition().SubChapterIndex + 1;
	if (!State->GetDocument().SubChapters.IsValidIndex(NextIndex))
	{
		bNarrationRequested = false;
		return;
	}

	bAdvancingInternally = true;
	State->NextSubChapter();
	bAdvancingInternally = false;

	PlayNarration();
}

void UBookAudioDirectorComponent::HandlePositionChanged(const FBookPosition& NewPosition)
{
	if (bAdvancingInternally || !bNarrationRequested)
	{
		return;
	}

	PlayNarration();
}

void UBookAudioDirectorComponent::HandleModeChanged(EExperienceMode NewMode)
{
	RefreshForMode();
}
