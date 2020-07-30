#include "GeneralConfig.h"

#include "MacroConfig.h"

#include "ThrowException.h"
#include "XmlReader.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>


namespace
{
	Storm::LogLevel logLevelFromString(std::string &&levelStr)
	{
		boost::to_lower(levelStr);

#define STORM_RETURN_IF_LEVEL_IS(LogLevelEnum) if (levelStr == boost::to_lower_copy(std::string{ #LogLevelEnum })) return Storm::LogLevel::LogLevelEnum

		STORM_RETURN_IF_LEVEL_IS(Debug);
		STORM_RETURN_IF_LEVEL_IS(DebugError);
		STORM_RETURN_IF_LEVEL_IS(Comment);
		STORM_RETURN_IF_LEVEL_IS(Fatal);
		//STORM_RETURN_IF_LEVEL_IS(Always); => We shouldn't remove Fatal logging! "Always" is greater than "Fatal" so it shouldn't be considered as a threshold.
		STORM_RETURN_IF_LEVEL_IS(Warning);
		STORM_RETURN_IF_LEVEL_IS(Error);

#undef STORM_RETURN_IF_LEVEL_IS

		Storm::throwException<std::exception>("Unknown Log level : " + levelStr);
	}
}


Storm::GeneralConfig::GeneralConfig() :
	_logLevel{ Storm::LogLevel::Debug },
	_overrideLogs{ true },
	_removeLogsOlderThanDays{ -1 },
	_shouldLogFPSWatching{ false },
	_wantedApplicationHeight{ 800 },
	_wantedApplicationWidth{ 1200 },
	_fontSize{ 20.f },
	_shouldLogGraphicDeviceMessage{ false }
{

}

bool Storm::GeneralConfig::read(const std::string &generalConfigFilePathStr)
{
	const std::filesystem::path generalConfigFilePath{ generalConfigFilePathStr };
	if (std::filesystem::is_regular_file(generalConfigFilePath))
	{
		if (generalConfigFilePath.extension() == ".xml")
		{
			boost::property_tree::ptree xmlTree;
			boost::property_tree::read_xml(generalConfigFilePathStr, xmlTree, boost::property_tree::xml_parser::no_comments);

			const auto &generalTree = xmlTree.get_child("General");

			/* Logs */
			const auto &logTreeOpt = generalTree.get_child_optional("Log");
			if (logTreeOpt.has_value())
			{
				const auto &logTree = logTreeOpt.value();
				for (const auto &logXmlElement : logTree)
				{
					if (
						!Storm::XmlReader::handleXml(logXmlElement, "logFolderPath", _logFolderPath) &&
						!Storm::XmlReader::handleXml(logXmlElement, "logFileName", _logFileName) &&
						!Storm::XmlReader::handleXml(logXmlElement, "logLevel", _logLevel, logLevelFromString) &&
						!Storm::XmlReader::handleXml(logXmlElement, "override", _overrideLogs) &&
						!Storm::XmlReader::handleXml(logXmlElement, "fpsWatching", _shouldLogFPSWatching) &&
						!Storm::XmlReader::handleXml(logXmlElement, "logGraphicDeviceMessage", _shouldLogGraphicDeviceMessage) &&
						!Storm::XmlReader::handleXml(logXmlElement, "removeOlderThanDays", _removeLogsOlderThanDays)
						)
					{
						LOG_ERROR << logXmlElement.first << " (inside General.Log) is unknown, therefore it cannot be handled";
					}
				}
			}

			const auto &graphicTreeOpt = generalTree.get_child_optional("Graphics");
			if (graphicTreeOpt.has_value())
			{
				const auto &graphicTree = graphicTreeOpt.value();
				for (const auto &graphicXmlElement : graphicTree)
				{
					if (
						!Storm::XmlReader::handleXml(graphicXmlElement, "screenWidth", _wantedApplicationWidth) &&
						!Storm::XmlReader::handleXml(graphicXmlElement, "screenHeight", _wantedApplicationHeight) &&
						!Storm::XmlReader::handleXml(graphicXmlElement, "fontSize", _fontSize)
						)
					{
						LOG_ERROR << graphicXmlElement.first << " (inside General.Graphics) is unknown, therefore it cannot be handled";
					}
				}
			}

			if (_wantedApplicationWidth == 0)
			{
				Storm::throwException<std::exception>("screenWidth cannot be null!");
			}
			else if (_wantedApplicationHeight == 0)
			{
				Storm::throwException<std::exception>("screenHeight cannot be null!");
			}
			else if (_fontSize <= 0)
			{
				Storm::throwException<std::exception>("Font size must be a strictly positive value! Current value was " + std::to_string(_fontSize));
			}

			return true;
		}
	}

	return false;
}

void Storm::GeneralConfig::applyMacros(const Storm::MacroConfig &macroConf)
{
	macroConf(_logFolderPath);
	macroConf(_logFileName);

	_logFileName = std::filesystem::path{ _logFileName }.filename().string();
}
