#include "BookSettings.h"

FString UBookSettings::GetPdfUrl()
{
	return GetDefault<UBookSettings>()->PdfUrl;
}

FString UBookSettings::GetStreamBaseUrl()
{
	return GetDefault<UBookSettings>()->StreamBaseUrl;
}

EExperienceMode UBookSettings::GetDefaultMode()
{
	return GetDefault<UBookSettings>()->DefaultMode;
}
