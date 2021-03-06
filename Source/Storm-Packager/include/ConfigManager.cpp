#include "ConfigManager.h"

#include "XmlReader.h"

#include "StormPathHelper.h"

#include <boost\program_options\variables_map.hpp>
#include <boost\program_options\parsers.hpp>
#include <boost\program_options\options_description.hpp>

#include <boost\property_tree\ptree_fwd.hpp>
#include <boost\property_tree\xml_parser.hpp>


#define STORM_XMACRO_COMMANDLINE \
STORM_XMACRO_COMMANDLINE_ELEM("build", std::string, std::string{}, "The branch to build before packaging. If there is none, no build will be made.", getBuildBranch)

namespace
{
	static boost::program_options::variables_map g_commandlineMap;

	template<class Type>
	bool extractIfExist(const std::string &val, Type &outVar)
	{
		if (g_commandlineMap.count(val))
		{
			outVar = g_commandlineMap[val].as<Type>();
			return true;
		}

		return false;
	}

	void readPackagerConfigFile(const std::filesystem::path &configFilePath, std::vector<std::string> &inOutCopyVect)
	{
		LOG_DEBUG << "Reading the packager config file located at " << configFilePath;

		const std::string configFilePathStr = Storm::toStdString(configFilePath);
		if (!std::filesystem::is_regular_file(configFilePath))
		{
			Storm::throwException<Storm::Exception>(configFilePathStr + " doesn't exist or isn't a file!");
		}

		boost::property_tree::ptree xmlTree;
		boost::property_tree::read_xml(configFilePathStr, xmlTree, boost::property_tree::xml_parser::no_comments);

		boost::property_tree::ptree child = xmlTree.get_child("Package");

		inOutCopyVect.reserve(child.size() + 1);

		inOutCopyVect.emplace_back();

		for (const auto &logicXmlElement : child)
		{
			std::string &toCopy = inOutCopyVect.back();

			if (!Storm::XmlReader::handleXml(logicXmlElement, "copy", toCopy))
			{
				LOG_ERROR << "tag '" << logicXmlElement.first << "' (inside Packaging.xml) is unknown, therefore it cannot be handled";
			}

			if (!toCopy.empty())
			{
				inOutCopyVect.emplace_back();
			}
		}

		inOutCopyVect.pop_back();

		if (inOutCopyVect.empty())
		{
			Storm::throwException<Storm::Exception>("Something went wrong when reading " + configFilePathStr);
		}
	}
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
	extractIfExist("help", _helpRequested);

	_currentExePath = argv[0];
	std::filesystem::path exePath{ _currentExePath };
	if (!exePath.is_absolute())
	{
		_currentExePath = (std::filesystem::current_path() / exePath.filename()).string();
		exePath = _currentExePath;
	}

	const std::filesystem::path rootPath = Storm::StormPathHelper::findStormRootPath(exePath.parent_path());
	_stormRootPath = rootPath.string();

	const std::filesystem::path destinationPackageFolderPath = rootPath / "Intermediate" / "Package";
	_destinationPackageFolderPath = destinationPackageFolderPath.string();

	_destinationPackageName = "Storm";

	readPackagerConfigFile(rootPath / "Config" / "Packaging" / "Packaging.xml", _toCopyPath);

	const std::filesystem::path tmpPath = std::filesystem::temp_directory_path() / "Storm_Packager";
	_tmpPath = tmpPath.string();
}

void StormPackager::ConfigManager::cleanUp_Implementation()
{
	const std::filesystem::path tmpPath = _tmpPath;
	if (std::filesystem::exists(tmpPath))
	{
		LOG_DEBUG << "Removing " << _tmpPath << " folder";
		std::filesystem::remove_all(tmpPath);
	}
}

#define STORM_XMACRO_COMMANDLINE_ELEM(tag, type, defaultValue, helpMsg, funcName)	\
type StormPackager::ConfigManager::funcName() const									\
{																					\
	type result = defaultValue;														\
	extractIfExist(tag, result);													\
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
	LOG_ALWAYS << "Storm-Packager help requested : \n" << _help;
}

const std::string& StormPackager::ConfigManager::getCurrentExePath() const noexcept
{
	return _currentExePath;
}

const std::string& StormPackager::ConfigManager::getStormRootPath() const noexcept
{
	return _stormRootPath;
}

const std::string& StormPackager::ConfigManager::getTmpPath() const noexcept
{
	return _tmpPath;
}

const std::string& StormPackager::ConfigManager::getDestinationPackageFolderPath() const noexcept
{
	return _destinationPackageFolderPath;
}

const std::string& StormPackager::ConfigManager::getDestinationPackageName() const noexcept
{
	return _destinationPackageName;
}

const std::vector<std::string>& StormPackager::ConfigManager::getToCopyPath() const noexcept
{
	return _toCopyPath;
}

