#include "CommandLineParser.h"
#include "ThrowException.h"


#define STORM_XMACRO_COMMANDLINE																																					\
STORM_XMACRO_COMMANDLINE_ELEM("scene", std::string, std::string{}, "The scene config file to use (path).", getSceneFilePath)														\
STORM_XMACRO_COMMANDLINE_ELEM("macroConfig", std::string, std::string{}, "The general config file to use (path).", getMacroConfigFilePath)											\
STORM_XMACRO_COMMANDLINE_ELEM("generalConfig", std::string, std::string{}, "The general config file to use (path).", getGeneralConfigFilePath)										\
STORM_XMACRO_COMMANDLINE_ELEM("tempPath", std::string, std::string{}, "The temporary path to use (path).", getTempPath)																\
STORM_XMACRO_COMMANDLINE_ELEM("recordMode", std::string, std::string{}, "The mode the simulator is launched into.", getRecordModeStr)												\
STORM_XMACRO_COMMANDLINE_ELEM("recordFile", std::string, std::string{}, "The path to the record file to write/read (path).", getRecordFilePath)										\
STORM_XMACRO_COMMANDLINE_ELEM("regenPCache", bool, false, "Force invalidating the particle cache data. Therefore regenerating all of them anew.", getShouldRegenerateParticleCache) \


Storm::CommandLineParser::CommandLineParser(int argc, const char* argv[]) :
	_shouldDisplayHelp{ false }
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
		_commandlineMap
	);

	boost::program_options::notify(_commandlineMap);

	_help = (std::stringstream{} << desc).str();
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

#define STORM_XMACRO_COMMANDLINE_ELEM(tag, type, defaultValue, helpMsg, funcName)	\
type Storm::CommandLineParser::funcName() const										\
{																					\
	type result = defaultValue;														\
	this->extractIfExist(tag, result);												\
	return result;																	\
}
STORM_XMACRO_COMMANDLINE
#undef STORM_XMACRO_COMMANDLINE_ELEM
