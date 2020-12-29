#pragma once


namespace Storm
{
	struct GeneratedGitConfig;
	struct InternalReferenceConfig;

	struct InternalConfig
	{
	public:
		InternalConfig();
		~InternalConfig();

	public:
		// Generated
		std::unique_ptr<Storm::GeneratedGitConfig> _generatedGitConfig;

		// Internal
		std::vector<Storm::InternalReferenceConfig> _internalReferencesConfig;
	};
}
