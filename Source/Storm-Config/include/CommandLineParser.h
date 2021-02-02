#pragma once

#include <boost/program_options.hpp>

namespace Storm
{
	enum class ThreadPriority;

	class CommandLineParser
	{
	public:
		CommandLineParser(int argc, const char* argv[]);

		const std::string& getHelp() const;
		bool shouldDisplayHelp() const;

		const std::string& getRawCommandline() const noexcept;

		// Getters

		std::string getSceneFilePath() const;
		std::string getTempPath() const;
		std::string getMacroConfigFilePath() const;
		std::string getGeneralConfigFilePath() const;

		std::string getRecordModeStr() const;
		std::string getRecordFilePath() const;

		std::string getStateFilePath() const;

		bool getShouldRegenerateParticleCache() const;

		bool getNoUI() const;

		bool noPhysicsTimeLoad() const;
		bool noVelocityLoad() const;
		bool noForceLoad() const;

		bool clearLogFolder() const;

		Storm::ThreadPriority getThreadPriority() const;

		// Tag getters

		std::string_view getSceneFilePathTag() const;
		std::string_view getTempPathTag() const;
		std::string_view getMacroConfigFilePathTag() const;
		std::string_view getGeneralConfigFilePathTag() const;

		std::string_view getRecordModeStrTag() const;
		std::string_view getRecordFilePathTag() const;

		std::string_view getStateFilePathTag() const;

		std::string_view getShouldRegenerateParticleCacheTag() const;

		std::string_view getNoUITag() const;

		std::string_view noPhysicsTimeLoadTag() const;
		std::string_view noVelocityLoadTag() const;
		std::string_view noForceLoadTag() const;

		std::string_view clearLogFolderTag() const;

		std::string_view getThreadPriorityTag() const;

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

		std::string _restartCommandline;
	};
}
