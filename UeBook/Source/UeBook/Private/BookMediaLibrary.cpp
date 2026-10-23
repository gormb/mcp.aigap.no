#include "BookMediaLibrary.h"

#include "BookSettings.h"
#include "Dom/JsonObject.h"
#include "MarkdownLibrary.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogBookMedia, Log, All);

namespace
{
	FString GetString(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, const FString& Fallback = FString())
	{
		FString Value;
		if (Object.IsValid() && Object->TryGetStringField(Field, Value))
		{
			return Value;
		}
		return Fallback;
	}

	float GetNumber(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, float Fallback = 0.0f)
	{
		double Value = 0.0;
		if (Object.IsValid() && Object->TryGetNumberField(Field, Value))
		{
			return static_cast<float>(Value);
		}
		return Fallback;
	}
}

FString UBookMediaLibrary::GetManifestPath(const FString& BookPath)
{
	const FString FullPath = UMarkdownLibrary::ResolveBookPath(BookPath);
	return FPaths::Combine(FPaths::GetPath(FullPath), FPaths::GetBaseFilename(FullPath) + TEXT(".media.json"));
}

bool UBookMediaLibrary::LoadManifest(const FString& BookPath, FBookMediaManifest& OutManifest)
{
	OutManifest = FBookMediaManifest();

	const UBookSettings* Settings = GetDefault<UBookSettings>();
	OutManifest.StreamBaseUrl = Settings->StreamBaseUrl;
	OutManifest.AudioExtension = Settings->AudioExtension.IsEmpty() ? TEXT("mp3") : Settings->AudioExtension;
	OutManifest.DefaultNarrator = Settings->DefaultNarratorId;
	OutManifest.PdfUrl = Settings->PdfUrl;
	OutManifest.BookId = FPaths::GetBaseFilename(UMarkdownLibrary::ResolveBookPath(BookPath));

	const FString ManifestPath = GetManifestPath(BookPath);
	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *ManifestPath))
	{
		// No sidecar: the settings above are enough to derive streaming URLs from subchapter slugs.
		return false;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogBookMedia, Warning, TEXT("Could not parse media manifest '%s'"), *ManifestPath);
		return false;
	}

	OutManifest.BookId = GetString(Root, TEXT("bookId"), OutManifest.BookId);
	OutManifest.Title = GetString(Root, TEXT("title"));
	OutManifest.PdfUrl = GetString(Root, TEXT("pdfUrl"), OutManifest.PdfUrl);
	OutManifest.StreamBaseUrl = GetString(Root, TEXT("streamBaseUrl"), OutManifest.StreamBaseUrl);
	OutManifest.AudioExtension = GetString(Root, TEXT("audioExtension"), OutManifest.AudioExtension);
	OutManifest.DefaultNarrator = GetString(Root, TEXT("defaultNarrator"), OutManifest.DefaultNarrator);

	const TArray<TSharedPtr<FJsonValue>>* Narrators = nullptr;
	if (Root->TryGetArrayField(TEXT("narrators"), Narrators))
	{
		for (const TSharedPtr<FJsonValue>& NarratorValue : *Narrators)
		{
			const TSharedPtr<FJsonObject> NarratorObject = NarratorValue->AsObject();
			if (!NarratorObject.IsValid())
			{
				continue;
			}

			FNarratorInfo Narrator;
			Narrator.Id = GetString(NarratorObject, TEXT("id"));
			Narrator.DisplayName = GetString(NarratorObject, TEXT("displayName"), Narrator.Id);
			Narrator.Language = GetString(NarratorObject, TEXT("language"));

			const TArray<TSharedPtr<FJsonValue>>* Clips = nullptr;
			if (NarratorObject->TryGetArrayField(TEXT("clips"), Clips))
			{
				for (const TSharedPtr<FJsonValue>& ClipValue : *Clips)
				{
					const TSharedPtr<FJsonObject> ClipObject = ClipValue->AsObject();
					if (!ClipObject.IsValid())
					{
						continue;
					}

					FNarrationClip Clip;
					Clip.SubChapter = GetString(ClipObject, TEXT("subChapter"));
					Clip.Url = GetString(ClipObject, TEXT("url"));
					Clip.DurationSec = GetNumber(ClipObject, TEXT("durationSec"));
					Narrator.Clips.Add(Clip);
				}
			}

			if (!Narrator.Id.IsEmpty())
			{
				OutManifest.Narrators.Add(Narrator);
			}
		}
	}

	UE_LOG(LogBookMedia, Log, TEXT("Loaded media manifest '%s' (%d narrator(s))"), *ManifestPath, OutManifest.Narrators.Num());
	return true;
}

bool UBookMediaLibrary::FindNarrator(const FBookMediaManifest& Manifest, const FString& NarratorId, FNarratorInfo& OutNarrator)
{
	const FString Wanted = NarratorId.IsEmpty() ? Manifest.DefaultNarrator : NarratorId;
	for (const FNarratorInfo& Narrator : Manifest.Narrators)
	{
		if (Narrator.Id.Equals(Wanted, ESearchCase::IgnoreCase))
		{
			OutNarrator = Narrator;
			return true;
		}
	}
	return false;
}

FString UBookMediaLibrary::ResolveNarrationUrl(
	const FBookMediaManifest& Manifest,
	const FString& NarratorId,
	int32 SubChapterIndex,
	const FBookSubChapter& SubChapter)
{
	FNarratorInfo Narrator;
	if (FindNarrator(Manifest, NarratorId, Narrator))
	{
		for (const FNarrationClip& Clip : Narrator.Clips)
		{
			const bool bIndexMatch = Clip.SubChapterIndex != INDEX_NONE && Clip.SubChapterIndex == SubChapterIndex;
			const bool bTitleMatch = Clip.SubChapterIndex == INDEX_NONE
				&& Clip.SubChapter.Equals(SubChapter.Title, ESearchCase::IgnoreCase);
			if (bIndexMatch || bTitleMatch)
			{
				return Clip.Url;
			}
		}
	}

	if (Manifest.StreamBaseUrl.IsEmpty() || SubChapter.Slug.IsEmpty())
	{
		return FString();
	}

	const FString NarratorPath = Narrator.Id.IsEmpty() ? Manifest.DefaultNarrator : Narrator.Id;
	if (NarratorPath.IsEmpty())
	{
		return FString();
	}

	const FString Extension = Manifest.AudioExtension.IsEmpty() ? TEXT("mp3") : Manifest.AudioExtension;
	return FString::Printf(TEXT("%s/%s/%s.%s"),
		*Manifest.StreamBaseUrl.TrimEnd(),
		*NarratorPath,
		*SubChapter.Slug,
		*Extension);
}

void UBookMediaLibrary::BindClipsToDocument(FBookMediaManifest& Manifest, const TArray<FBookSubChapter>& SubChapters)
{
	for (FNarratorInfo& Narrator : Manifest.Narrators)
	{
		for (FNarrationClip& Clip : Narrator.Clips)
		{
			Clip.SubChapterIndex = INDEX_NONE;
			for (int32 Index = 0; Index < SubChapters.Num(); ++Index)
			{
				if (SubChapters[Index].Title.Equals(Clip.SubChapter, ESearchCase::IgnoreCase))
				{
					Clip.SubChapterIndex = Index;
					break;
				}
			}
		}
	}
}
