#pragma once

#include "CoreMinimal.h"
#include "BookTypes.h"
#include "GameFramework/SaveGame.h"
#include "BookSaveGame.generated.h"

UCLASS()
class UEBOOK_API UBookSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString BookPath;

	UPROPERTY()
	FString BookId;

	UPROPERTY()
	int32 SubChapterIndex = 0;

	UPROPERTY()
	int32 PageNumber = 0;

	UPROPERTY()
	EExperienceMode Mode = EExperienceMode::Codex;

	UPROPERTY()
	FString NarratorId;
};
