#pragma once


namespace Storm
{
	enum class Language;

	struct GeneralApplicationConfig
	{
	public:
		GeneralApplicationConfig();

	public:
		bool _showBranchInTitle;
		bool _bipSoundOnFinish;
		Storm::Language _language;
	};
}
