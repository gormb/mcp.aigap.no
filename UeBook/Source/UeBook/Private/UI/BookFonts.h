#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"

namespace BookFonts
{
	/** EB Garamond when the imported font assets exist, engine default otherwise. */
	FSlateFontInfo Get(int32 Size, bool bBold = false, bool bItalic = false);
}
