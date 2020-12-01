#pragma once

#include <boost/program_options.hpp>

namespace Storm
{
	class CommandLineParser
	{
	public:
		CommandLineParser(int argc, const char* argv[]);

		const std::string& getHelp() const;
		bool shouldDisplayHelp() const;

		std::string getSceneFilePath() const;
		std::string getTempPath() const;
		std::string getMacroConfigFilePath() const;
		std::string getGeneralConfigFilePath() const;

		std::string getRecordModeStr() const;
		std::string getRecordFilePath() const;

		bool getShouldRegenerateParticleCache() const;

		bool getNoUI() const;

	public:
		template<class Type>
		bool extractIfExist(const std::string &val, Type &outVar) const
		{
			if (_commandlineMap.count(val))
			{
				outVar = _commandlineMap[val].as<Type>();
				return true;
			}

			return false;
		}

	private:
		boost::program_options::variables_map _commandlineMap;

		std::string _help;
		bool _shouldDisplayHelp;
		std::string _rawCommandLine;
	};
}
