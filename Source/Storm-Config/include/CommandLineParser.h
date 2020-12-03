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

	private:
		template<class Type>
		void extract(const std::string &val, Type &outVar) const
		{
			outVar = _commandlineMap[val].as<Type>();
		}

		template<class Type, class Converter>
		void extract(const std::string &val, Type &outVar, Converter &converter) const
		{
			outVar = converter(_commandlineMap[val].as<std::string>());
		}

	public:
		template<class Type, class ... MaybeConverter>
		bool extractIfExist(const std::string &val, Type &outVar, MaybeConverter ... converter) const
		{
			if (_commandlineMap.count(val))
			{
				this->extract<Type>(val, outVar, converter...);
				return true;
			}

			return false;
		}

		bool findIfExist(const std::string &val, bool noValue) const;

	private:
		boost::program_options::variables_map _commandlineMap;

		std::string _help;
		bool _shouldDisplayHelp;
		std::string _rawCommandLine;
	};
}
