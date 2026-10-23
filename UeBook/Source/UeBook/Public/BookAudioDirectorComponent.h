#pragma once

#include "CoreMinimal.h"
#include "BookTypes.h"
#include "Components/ActorComponent.h"
#include "BookAudioDirectorComponent.generated.h"

class UMediaPlayer;
class UMediaSoundComponent;

/**
 * Streams narration and music for the active experience mode and ducks music under narration.
 * Narration clips come from the manifest (or derived URLs); music comes from the project settings.
 */
UCLASS(ClassGroup = (Book), meta = (BlueprintSpawnableComponent))
class UEBOOK_API UBookAudioDirectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBookAudioDirectorComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Starts/stops narration and music to match the current mode. */
	void RefreshForMode();

	void PlayNarration();
	void StopNarration();
	void ToggleNarration();

	UFUNCTION(BlueprintPure, Category = "Book|Audio")
	bool IsNarrationPlaying() const;

	UFUNCTION(BlueprintPure, Category = "Book|Audio")
	FString GetNarrationUrl() const { return CurrentNarrationUrl; }

protected:
	UFUNCTION()
	void HandleNarrationFinished();

private:
	class UBookStateSubsystem* GetBookState() const;
	void StartMusic();
	void StopMusic();
	void HandlePositionChanged(const FBookPosition& NewPosition);
	void HandleModeChanged(EExperienceMode NewMode);

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> NarrationPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> MusicPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMediaSoundComponent> NarrationSound;

	UPROPERTY(Transient)
	TObjectPtr<UMediaSoundComponent> MusicSound;

	FString CurrentNarrationUrl;
	FString CurrentMusicUrl;
	float NarrationVolume = 0.0f;
	float MusicVolume = 0.0f;
	bool bNarrationRequested = false;
	bool bMusicRequested = false;
	bool bAdvancingInternally = false;
	bool bHandlesBound = false;
};
