#include "UI/BookFonts.h"

#include "Fonts/CompositeFont.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
	const TCHAR* RegularFontPath = TEXT("/Game/Fonts/EBGaramond_Regular.EBGaramond_Regular");
	const TCHAR* BoldFontPath = TEXT("/Game/Fonts/EBGaramond_Bold.EBGaramond_Bold");
	const TCHAR* ItalicFontPath = TEXT("/Game/Fonts/EBGaramond_Italic.EBGaramond_Italic");
	const TCHAR* BoldItalicFontPath = TEXT("/Game/Fonts/EBGaramond_BoldItalic.EBGaramond_BoldItalic");

	struct FLoadedFont
	{
		TStrongObjectPtr<UObject> Face;
		TSharedPtr<const FCompositeFont> Composite;
	};

	/** Slate resolves UFont objects, so layer the imported font faces into composite fonts ourselves. */
	FLoadedFont* LoadFont(const TCHAR* ObjectPath)
	{
		static TMap<FString, FLoadedFont> Cache;
		const FString Key(ObjectPath);

		if (FLoadedFont* Found = Cache.Find(Key))
		{
			return Found->Face.IsValid() ? Found : nullptr;
		}

		FLoadedFont& Loaded = Cache.Add(Key, FLoadedFont());
		Loaded.Face.Reset(LoadObject<UObject>(nullptr, ObjectPath));
		if (!Loaded.Face.IsValid())
		{
			return nullptr;
		}

		FCompositeFont Composite;
		FTypefaceEntry Entry(TEXT("Default"));
		Entry.Font = FFontData(Loaded.Face.Get());
		Composite.DefaultTypeface.Fonts.Add(Entry);

		// EB Garamond has no music-note glyph, so fall back to the engine font for anything it lacks.
		const FString RobotoPath = FPaths::Combine(FPaths::EngineContentDir(), TEXT("Slate/Fonts/Roboto-Regular.ttf"));
		if (FPaths::FileExists(RobotoPath))
		{
			FTypefaceEntry FallbackEntry(TEXT("Fallback"));
			FallbackEntry.Font = FFontData(RobotoPath, EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
			Composite.FallbackTypeface.Typeface.Fonts.Add(FallbackEntry);
		}

		Loaded.Composite = MakeShared<const FCompositeFont>(Composite);

		return &Loaded;
	}
}

FSlateFontInfo BookFonts::Get(int32 Size, bool bBold, bool bItalic)
{
	const TCHAR* Path = RegularFontPath;
	if (bBold && bItalic)
	{
		Path = BoldItalicFontPath;
	}
	else if (bBold)
	{
		Path = BoldFontPath;
	}
	else if (bItalic)
	{
		Path = ItalicFontPath;
	}

	if (const FLoadedFont* Loaded = LoadFont(Path))
	{
		if (Loaded->Composite.IsValid())
		{
			return FSlateFontInfo(Loaded->Composite, Size);
		}
	}

	FName Typeface = TEXT("Regular");
	if (bBold && bItalic)
	{
		Typeface = TEXT("BoldItalic");
	}
	else if (bBold)
	{
		Typeface = TEXT("Bold");
	}
	else if (bItalic)
	{
		Typeface = TEXT("Italic");
	}

	return FCoreStyle::GetDefaultFontStyle(Typeface, Size);
}
