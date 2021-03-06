#include "CommandLineParser.h"

#include "ThreadPriority.h"

#include <boost\algorithm\string\case_conv.hpp>


#define STORM_XMACRO_COMMANDLINE																																										\
STORM_XMACRO_COMMANDLINE_ELEM("scene", std::string, std::string{}, "The scene config file to use (path).", getSceneFilePath)																			\
STORM_XMACRO_COMMANDLINE_ELEM("macroConfig", std::string, std::string{}, "The general config file to use (path).", getMacroConfigFilePath)																\
STORM_XMACRO_COMMANDLINE_ELEM("generalConfig", std::string, std::string{}, "The general config file to use (path).", getGeneralConfigFilePath)															\
STORM_XMACRO_COMMANDLINE_ELEM("tempPath", std::string, std::string{}, "The temporary path to use (path).", getTempPath)																					\
STORM_XMACRO_COMMANDLINE_ELEM("mode", std::string, std::string{}, "The mode the simulator is launched into.", getRecordModeStr)																			\
STORM_XMACRO_COMMANDLINE_ELEM("recordFile", std::string, std::string{}, "The path to the record file to write/read (path).", getRecordFilePath)															\
STORM_XMACRO_COMMANDLINE_ELEM("stateFile", std::string, std::string{}, "The path to the simulation state file to load from (path).", getStateFilePath)													\
STORM_XMACRO_COMMANDLINE_ELEM("threadPriority", Storm::ThreadPriority, Storm::ThreadPriority::Unset, "Simulation thread priority between Normal/Below/High.", getThreadPriority, parseThreadPriority)	\
STORM_XMACRO_COMMANDLINE_ELEM_NO_VALUE("regenPCache", false, "Force invalidating the particle cache data. Therefore regenerating all of them anew.", getShouldRegenerateParticleCache)					\
STORM_XMACRO_COMMANDLINE_ELEM_NO_VALUE("noUI", false, "Setting it specify that we shouldn't start the UI and only focus on the simulation on the background.", getNoUI)									\
STORM_XMACRO_COMMANDLINE_ELEM_NO_VALUE("noPhysicsTimeLoad", false, "Setting it specify that we shouldn't load physics time from state file (time will be set to the time 0).", noPhysicsTimeLoad)		\
STORM_XMACRO_COMMANDLINE_ELEM_NO_VALUE("noVelocityLoad", false, "Setting it specify that we shouldn't load velocities from state file (velocities will be set to null vector).", noVelocityLoad)		\
STORM_XMACRO_COMMANDLINE_ELEM_NO_VALUE("noForceLoad", false, "Setting it specify that we shouldn't load forces from state file (forces will be set to null vector).", noForceLoad)						\
STORM_XMACRO_COMMANDLINE_ELEM_NO_VALUE("clearLogs", false, "Flag to specify we must clear all logs. Warning: it shouldn't be used when multiple Storm processes are running.", clearLogFolder)			\


namespace
{
	struct CommandLinePolicy
	{
	public:
		static std::string parsePolicyAgnostic(const std::vector<std::string_view> &args)
		{
			std::string result;

			std::size_t totalCount = 0;
			for (const std::string_view &arg : args)
			{
				totalCount += arg.size();
			}

			if (totalCount > 0)
			{
				result.reserve(totalCount + (args.size() * 3));

				for (const std::string_view &arg : args)
				{
					if (!arg.empty())
					{
						result += '"';
						result += arg;
						result += "\" ";
					}
				}

				result.pop_back();
			}

			return result;
		}
	};

	template<class Type> struct CmdInterfaceTypeExtractor { using InterfacingType = Type; };
	template<> struct CmdInterfaceTypeExtractor<Storm::ThreadPriority> { using InterfacingType = std::string; };

	Storm::ThreadPriority parseThreadPriority(std::string threadPriorityStr)
	{
		boost::algorithm::to_lower(threadPriorityStr);
		if (threadPriorityStr == "below")
		{
			return Storm::ThreadPriority::Below;
		}
		else if (threadPriorityStr == "normal")
		{
			return Storm::ThreadPriority::Normal;
		}
		else if (threadPriorityStr == "high")
		{
			return Storm::ThreadPriority::High;
		}
		else
		{
			Storm::throwException<Storm::Exception>("Unknown thread priority");
		}
	}
}

Storm::CommandLineParser::CommandLineParser(int argc, const char* argv[]) :
	_shouldDisplayHelp{ false },
	_rawCommandLine{ Storm::toStdString<CommandLinePolicy>(std::vector<std::string_view>{ argv, argv + argc }) }
{
	LOG_COMMENT << "Parsing command line :\n" << _rawCommandLine;

	boost::program_options::options_description desc{ "Options" };
	desc.add_options()
		("help,h", "Command line help")
#define STORM_XMACRO_COMMANDLINE_ELEM(tag, type, defaultValue, helpMsg, funcName, ...) (tag, boost::program_options::value<CmdInterfaceTypeExtractor<type>::InterfacingType>(), helpMsg)
#define STORM_XMACRO_COMMANDLINE_ELEM_NO_VALUE(tag, defaultValue, helpMsg, funcName, ...) (tag, helpMsg)
		STORM_XMACRO_COMMANDLINE
#undef STORM_XMACRO_COMMANDLINE_ELEM
#undef STORM_XMACRO_COMMANDLINE_ELEM_NO_VALUE
		;

	boost::program_options::store(
		boost::program_options::command_line_parser{ argc, argv }
			.options(desc)
			.style(boost::program_options::command_line_style::unix_style | boost::program_options::command_line_style::case_insensitive).run(),
		_commandlineMap
	);

	boost::program_options::notify(_commandlineMap);

	_help = Storm::toStdString(desc);
	this->extractIfExist("help", _shouldDisplayHelp);
}

const std::string& Storm::CommandLineParser::getHelp() const
{
	return _help;
}

bool Storm::CommandLineParser::shouldDisplayHelp() const
{
	return _shouldDisplayHelp;
}

const std::string& Storm::CommandLineParser::getRawCommandline() const noexcept
{
	return _rawCommandLine;
}

bool Storm::CommandLineParser::findIfExist(const std::string &val, bool noValue) const
{
	return _commandlineMap.count(val) ? !noValue : noValue;
}

#define STORM_XMACRO_COMMANDLINE_ELEM(tag, type, defaultValue, helpMsg, funcName, ...)	\
type Storm::CommandLineParser::funcName() const											\
{																						\
	type result = defaultValue;															\
	this->extractIfExist(tag, result, __VA_ARGS__);										\
	return result;																		\
}
#define STORM_XMACRO_COMMANDLINE_ELEM_NO_VALUE(tag, defaultValue, helpMsg, funcName, ...) 	\
bool Storm::CommandLineParser::funcName() const												\
{																							\
	return this->findIfExist(tag, defaultValue);											\
}
STORM_XMACRO_COMMANDLINE
#undef STORM_XMACRO_COMMANDLINE_ELEM
#undef STORM_XMACRO_COMMANDLINE_ELEM_NO_VALUE


#define STORM_XMACRO_COMMANDLINE_ELEM(tag, type, defaultValue, helpMsg, funcName, ...) std::string_view Storm::CommandLineParser::funcName##Tag() const { return "--" tag; }
#define STORM_XMACRO_COMMANDLINE_ELEM_NO_VALUE(tag, defaultValue, helpMsg, funcName, ...) std::string_view Storm::CommandLineParser::funcName##Tag() const { return "--" tag; }
STORM_XMACRO_COMMANDLINE
#undef STORM_XMACRO_COMMANDLINE_ELEM
#undef STORM_XMACRO_COMMANDLINE_ELEM_NO_VALUE
