#pragma once

#include "Singleton.h"


namespace StormPackager
{
	class IPackagingLogic;

	class PackagerManager : private Storm::Singleton<StormPackager::PackagerManager>
	{
		STORM_DECLARE_SINGLETON(PackagerManager);

	private:
		using PackagerContainer = std::vector<std::unique_ptr<StormPackager::IPackagingLogic>>;

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		bool run();

	private:
		PackagerContainer _packagerList;
		bool _prepared;
	};
}
