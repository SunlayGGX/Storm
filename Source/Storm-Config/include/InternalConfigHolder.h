#pragma once


namespace Storm
{
	class MacroConfigHolder;
	struct InternalConfig;

	class InternalConfigHolder
	{
	public:
		InternalConfigHolder();
		~InternalConfigHolder();

	public:
		// Produce the generated configs
		void init();

		// Read from internal configs
		void read(const std::filesystem::path &internalConfigPath, const Storm::MacroConfigHolder &macroConfig);

	public:
		const Storm::InternalConfig& getInternalConfig() const;

	private:
		std::unique_ptr<Storm::InternalConfig> _internalConfig;
	};
}
