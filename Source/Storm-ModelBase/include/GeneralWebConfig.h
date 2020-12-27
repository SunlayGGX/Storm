#pragma once


namespace Storm
{
	enum class PreferredBrowser;

	struct GeneralWebConfig
	{
	public:
		GeneralWebConfig();

	public:
		bool _urlOpenIncognito;
		Storm::PreferredBrowser _preferredBrowser;
	};
}
