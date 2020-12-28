#pragma once


namespace Storm
{
	struct GeneratedGitConfig;

	struct InternalConfig
	{
	public:
		InternalConfig();
		~InternalConfig();

	public:
		std::unique_ptr<Storm::GeneratedGitConfig> _generatedGitConfig;
	};
}
