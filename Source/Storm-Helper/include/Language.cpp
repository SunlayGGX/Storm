#include "Language.h"

#include "LeanWindowsInclude.h"

#include <boost\algorithm\string\case_conv.hpp>


Storm::Language Storm::parseLanguage(std::string languageStr)
{
	boost::to_lower(languageStr);
	if (languageStr == "english")
	{
		return Storm::Language::English;
	}
	else if (languageStr == "french")
	{
		return Storm::Language::French;
	}
	else
	{
		Storm::throwException<Storm::Exception>("Unknown or unhandled language : " + languageStr);
	}
}

Storm::Language Storm::retrieveDefaultOSLanguage()
{
	WCHAR name[LOCALE_NAME_MAX_LENGTH];
	if (int languageSz = ::GetUserDefaultLocaleName(name, LOCALE_NAME_MAX_LENGTH); languageSz > 0)
	{
		std::wstring_view language{ name, static_cast<std::size_t>(languageSz - 1) };
		language = language.substr(0, language.find('-'));
		if (language == L"en")
		{
			return Storm::Language::English;
		}
		else if (language == L"fr")
		{
			return Storm::Language::French;
		}
		else
		{
			assert(false && "Unhandled OS Language. Falling back to English but some feature won't work correctly if not set manually by user!");
			return Storm::Language::English;
		}
	}
	else
	{
		LOG_ERROR << "Cannot retrieve the default OS Language. Error was " << GetLastError() << ". Falling back to English.";
		return Storm::Language::English;
	}
}
