#pragma once

#include "Singleton.h"


namespace StormPackager
{
	class ConfigManager : private Storm::Singleton<StormPackager::ConfigManager>
	{
		STORM_DECLARE_SINGLETON(ConfigManager);

	private:
		void initialize_Implementation(int argc, const char*const argv[]);
		void cleanUp_Implementation();

	public:
		bool helpRequested() const noexcept;
		void printHelp() const;

		const std::string& getCurrentExePath() const noexcept;
		const std::string& getStormRootPath() const noexcept;
		const std::string& getTmpPath() const noexcept;
		const std::string& getDestinationPackageFolderPath() const noexcept;
		const std::string& getDestinationPackageName() const noexcept;
		const std::vector<std::string>& getToCopyPath() const noexcept;

	private:
		bool _helpRequested;
		std::string _help;

		std::string _currentExePath;
		std::string _stormRootPath;
		std::string _tmpPath;
		std::string _destinationPackageFolderPath;
		std::string _destinationPackageName;
		std::vector<std::string> _toCopyPath;
	};
}
