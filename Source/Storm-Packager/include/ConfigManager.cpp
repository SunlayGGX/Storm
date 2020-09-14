#include "ConfigManager.h"

#include "ThrowException.h"

#include <boost\program_options\variables_map.hpp>
#include <boost\program_options\parsers.hpp>
#include <boost\program_options\options_description.hpp>

#include <iostream>


#define STORM_XMACRO_COMMANDLINE
//STORM_XMACRO_COMMANDLINE_ELEM("scene", std::string, std::string{}, "The scene config file to use (path).", getSceneFilePath)															\
//STORM_XMACRO_COMMANDLINE_ELEM("macroConfig", std::string, std::string{}, "The general config file to use (path).", getMacroConfigFilePath)											\
//STORM_XMACRO_COMMANDLINE_ELEM("generalConfig", std::string, std::string{}, "The general config file to use (path).", getGeneralConfigFilePath)										\
//STORM_XMACRO_COMMANDLINE_ELEM("tempPath", std::string, std::string{}, "The temporary path to use (path).", getTempPath)																\
//STORM_XMACRO_COMMANDLINE_ELEM("regenPCache", bool, false, "Force invalidating the particle cache data. Therefore regenerating all of them anew.", getShouldRegenerateParticleCache)	\


namespace
{
	template<class CommandLineMap, class Type>
	bool extractIfExist(const CommandLineMap &commandlineMap, const std::string &val, Type &outVar)
	{
		if (commandlineMap.count(val))
		{
			outVar = commandlineMap[val].as<Type>();
			return true;
		}

		return false;
	}

	static boost::program_options::variables_map g_commandlineMap;
}


StormPackager::ConfigManager::ConfigManager() :
	_helpRequested{ false }
{

}

StormPackager::ConfigManager::~ConfigManager() = default;

void StormPackager::ConfigManager::initialize_Implementation(int argc, const char*const argv[])
{
	boost::program_options::options_description desc{ "Options" };
	desc.add_options()
		("help,h", "Command line help")
#define STORM_XMACRO_COMMANDLINE_ELEM(tag, type, defaultValue, helpMsg, funcName) (tag, boost::program_options::value<type>(), helpMsg)
		STORM_XMACRO_COMMANDLINE
#undef STORM_XMACRO_COMMANDLINE_ELEM
		;

	boost::program_options::store(
		boost::program_options::command_line_parser{ argc, argv }
			.options(desc)
			.style(boost::program_options::command_line_style::unix_style | boost::program_options::command_line_style::case_insensitive).run(),
		g_commandlineMap
	);

	boost::program_options::notify(g_commandlineMap);

	_help = (std::stringstream{} << desc).str();
	extractIfExist(g_commandlineMap, "help", _helpRequested);
}

#define STORM_XMACRO_COMMANDLINE_ELEM(tag, type, defaultValue, helpMsg, funcName)	\
type StormPackager::ConfigManager::funcName() const									\
{																					\
	type result = defaultValue;														\
	extractIfExist(g_commandlineMap, tag, result);									\
	return result;																	\
}
STORM_XMACRO_COMMANDLINE
#undef STORM_XMACRO_COMMANDLINE_ELEM

bool StormPackager::ConfigManager::helpRequested() const noexcept
{
	return _helpRequested;
}

void StormPackager::ConfigManager::printHelp() const
{
	std::cout << "Storm-Packager help requested : \n" << _help;
}

