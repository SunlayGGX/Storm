#include "CommandLineParser.h"
#include "ThrowException.h"

#include <filesystem>


Storm::CommandLineParser::CommandLineParser(int argc, const char* argv[]) :
    _shouldDisplayHelp{ false }
{
    boost::program_options::options_description desc{ "Options" };
    desc.add_options()
        ("help,h", "Command line help")
        ("scene", boost::program_options::value<std::string>()->required(), "The scene config file to use.")
        ("inputBindings", boost::program_options::value<std::string>()->required(), "The input key binding config file to use.")
        ("tempPath", boost::program_options::value<std::string>()->default_value((std::filesystem::current_path().parent_path() / "Intermediate").string()), "The temporary path to use.")
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

std::string Storm::CommandLineParser::getSceneFilePath() const
{
    std::string result;
    this->extractIfExist("scene", result);
    return result;
}

std::string Storm::CommandLineParser::getInputConfigFilePath() const
{
    std::string result;
    this->extractIfExist("inputBindings", result);
    return result;
}

std::string Storm::CommandLineParser::getTempPath() const
{
    std::string result;
    this->extractIfExist("tempPath", result);
    return result;
}
