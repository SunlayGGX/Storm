#pragma once


namespace Storm
{
	enum class Language
	{
		English,
		French,
	};

	Storm::Language parseLanguage(std::string languageStr);
	Storm::Language retrieveDefaultOSLanguage();
}
