#pragma once

#include "CoreMinimal.h"
#include "BookTypes.h"
#include "Engine/DeveloperSettings.h"
#include "BookSettings.generated.h"

/** Project settings for sources, streamed audio and the default experience mode. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "UeBook"))
class UEBOOK_API UBookSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Opened by the Link mode and the L key. Falls back to the markdown source path. */
	UPROPERTY(Config, EditAnywhere, Category = "Source")
	FString PdfUrl;

	/** Base URL for streamed narration, e.g. https://media.example.com/lifedemandeddeath */
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	FString StreamBaseUrl;

	/** Optional looping music stream used by the Score, Sound Book and Studio modes. */
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	FString MusicStreamUrl;

	/** Narrator id used when the manifest does not name one. */
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	FString DefaultNarratorId = TEXT("author");

	/** Extension used when narration URLs are derived instead of listed in the manifest. */
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	FString AudioExtension = TEXT("mp3");

	UPROPERTY(Config, EditAnywhere, Category = "Reading")
	EExperienceMode DefaultMode = EExperienceMode::Codex;

	UFUNCTION(BlueprintPure, Category = "Book")
	static FString GetPdfUrl();

	UFUNCTION(BlueprintPure, Category = "Book")
	static FString GetStreamBaseUrl();

	UFUNCTION(BlueprintPure, Category = "Book")
	static EExperienceMode GetDefaultMode();

	virtual FName GetCategoryName() const override { return TEXT("Game"); }
};
