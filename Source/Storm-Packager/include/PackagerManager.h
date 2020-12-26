#pragma once

#include "Singleton.h"


namespace StormPackager
{
	class ITaskLogic;

	class PackagerManager final : private Storm::Singleton<StormPackager::PackagerManager>
	{
		STORM_DECLARE_SINGLETON(PackagerManager);

	private:
		using PackagerContainer = std::vector<std::unique_ptr<StormPackager::ITaskLogic>>;

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	public:
		bool run();

	private:
		PackagerContainer _taskList;
		bool _prepared;
	};
}
