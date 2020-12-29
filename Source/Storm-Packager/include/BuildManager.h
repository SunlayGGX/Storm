#pragma once

#include "Singleton.h"


namespace StormPackager
{
	class BuildProcess;

	class BuildManager final :
		private Storm::Singleton<StormPackager::BuildManager>
	{
		STORM_DECLARE_SINGLETON(BuildManager);

	private:
		bool initialize_Implementation();
		void cleanUp_Implementation();

	public:
		bool run();

	private:
		std::string _visualStudioLocation;

		std::string _branchToBuild;
		std::string _oldBranch;
		std::unique_ptr<StormPackager::BuildProcess> _buildProcess;
	};
}
