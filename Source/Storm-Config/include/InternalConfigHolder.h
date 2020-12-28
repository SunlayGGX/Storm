#pragma once


namespace Storm
{
	struct InternalConfig;

	class InternalConfigHolder
	{
	public:
		InternalConfigHolder();
		~InternalConfigHolder();

	public:
		void init();

	public:
		const Storm::InternalConfig& getInternalConfig() const;

	private:
		std::unique_ptr<Storm::InternalConfig> _internalConfig;
	};
}
