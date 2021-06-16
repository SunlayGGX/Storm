#include "GeneralConfigHolder.h"

#include "MacroConfigHolder.h"

#include "GeneralConfig.h"
#include "GeneralWebConfig.h"

#include "XmlReader.h"

#include "SocketSetting.h"

#include "VectoredExceptionDisplayMode.h"
#include "PreferredBrowser.h"

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
		STORM_RETURN_IF_LEVEL_IS(DebugWarning);
		STORM_RETURN_IF_LEVEL_IS(DebugError);
		STORM_RETURN_IF_LEVEL_IS(Comment);
		STORM_RETURN_IF_LEVEL_IS(Fatal);
		//STORM_RETURN_IF_LEVEL_IS(Always); => We shouldn't remove Fatal logging! "Always" is greater than "Fatal" so it shouldn't be considered as a threshold.
		STORM_RETURN_IF_LEVEL_IS(Warning);
		STORM_RETURN_IF_LEVEL_IS(Error);

#undef STORM_RETURN_IF_LEVEL_IS

		Storm::throwException<Storm::Exception>("Unknown Log level : " + levelStr);
	}

	Storm::VectoredExceptionDisplayMode parseVectoredExceptionDisplayMode(std::string valueStr)
	{
		boost::to_lower(valueStr);
		if (valueStr == "none")
		{
			return Storm::VectoredExceptionDisplayMode::None;
		}
		else if (valueStr == "fatalonly")
		{
			return Storm::VectoredExceptionDisplayMode::DisplayFatal;
		}
		else if (valueStr == "all")
		{
			return Storm::VectoredExceptionDisplayMode::DisplayAll;
		}
		else
		{
			Storm::throwException<Storm::Exception>("Unknown vectored exception display mode requested : " + valueStr);
		}
	}

	Storm::PreferredBrowser parsePreferredBrowser(std::string valueStr)
	{
		if (valueStr.empty())
		{
			return Storm::PreferredBrowser::None;
		}

		boost::to_lower(valueStr);
		if (valueStr == "chrome")
		{
			return Storm::PreferredBrowser::Chrome;
		}
		else if (valueStr == "firefox")
		{
			return Storm::PreferredBrowser::Firefox;
		}
		else if (valueStr == "edge")
		{
			return Storm::PreferredBrowser::Edge;
		}
		else if (valueStr == "internetexplorer")
		{
			return Storm::PreferredBrowser::InternetExplorer;
		}
		else
		{
			Storm::throwException<Storm::Exception>("Unknown or unhandled internet browser : " + valueStr);
		}
	}

	void parseSocketSettings(const boost::property_tree::ptree &tree, const std::unique_ptr<Storm::SocketSetting> &outSetting)
	{
		std::string ipStr;

		if (Storm::XmlReader::readXmlAttribute(tree, ipStr, "ip"))
		{
			outSetting->setIP(ipStr);
		}

		Storm::XmlReader::readXmlAttribute(tree, outSetting->_port, "port");

		if (!outSetting->isValid())
		{
			Storm::throwException<Storm::Exception>(Storm::toStdString(outSetting) + " is not a valid address!");
		}
		
		if (!Storm::XmlReader::readXmlAttribute(tree, outSetting->_isEnabled, "enabled"))
		{
			outSetting->_isEnabled = true;
		}
	}
}


Storm::GeneralConfigHolder::GeneralConfigHolder() :
	_generalConfig{ std::make_unique<Storm::GeneralConfig>() }
{

}

bool Storm::GeneralConfigHolder::read(const std::string &generalConfigFilePathStr)
{
	const std::filesystem::path generalConfigFilePath{ generalConfigFilePathStr };
	if (std::filesystem::is_regular_file(generalConfigFilePath))
	{
		if (generalConfigFilePath.extension() == ".xml")
		{
			boost::property_tree::ptree xmlTree;
			boost::property_tree::read_xml(generalConfigFilePathStr, xmlTree, boost::property_tree::xml_parser::no_comments);

			const auto &generalTree = xmlTree.get_child("General");

			/* Application */
			Storm::GeneralApplicationConfig &generalApplicationConfig = _generalConfig->_generalApplicationConfig;
			const auto &applicationTreeOpt = generalTree.get_child_optional("Application");
			if (applicationTreeOpt.has_value())
			{
				const auto &applicationTree = applicationTreeOpt.value();
				for (const auto &applicationXmlElement : applicationTree)
				{
					if (
						!Storm::XmlReader::handleXml(applicationXmlElement, "displayBranch", generalApplicationConfig._showBranchInTitle) &&
						!Storm::XmlReader::handleXml(applicationXmlElement, "beepOnFinish", generalApplicationConfig._bipSoundOnFinish)
						)
					{
						LOG_ERROR << applicationXmlElement.first << " (inside General.Application) is unknown, therefore it cannot be handled";
					}
				}
			}


			/* Graphic */
			Storm::GeneralGraphicConfig &generalGraphicConfig = _generalConfig->_generalGraphicConfig;

			const auto &graphicTreeOpt = generalTree.get_child_optional("Graphics");
			if (graphicTreeOpt.has_value())
			{
				const auto &graphicTree = graphicTreeOpt.value();
				for (const auto &graphicXmlElement : graphicTree)
				{
					if (
						!Storm::XmlReader::handleXml(graphicXmlElement, "screenWidth", generalGraphicConfig._wantedApplicationWidth) &&
						!Storm::XmlReader::handleXml(graphicXmlElement, "screenHeight", generalGraphicConfig._wantedApplicationHeight) &&
						!Storm::XmlReader::handleXml(graphicXmlElement, "screenX", generalGraphicConfig._wantedApplicationXPos) &&
						!Storm::XmlReader::handleXml(graphicXmlElement, "screenY", generalGraphicConfig._wantedApplicationYPos) &&
						!Storm::XmlReader::handleXml(graphicXmlElement, "nearFarPlaneFixed", generalGraphicConfig._fixNearFarPlanesWhenTranslating) &&
						!Storm::XmlReader::handleXml(graphicXmlElement, "selectedParticleAlwaysOnTop", generalGraphicConfig._selectedParticleShouldBeTopMost) &&
						!Storm::XmlReader::handleXml(graphicXmlElement, "selectedParticleForceAlwaysOnTop", generalGraphicConfig._selectedParticleForceShouldBeTopMost) &&
						!Storm::XmlReader::handleXml(graphicXmlElement, "spinCameraToGravity", generalGraphicConfig._spinCameraToGravityUp) &&
						!Storm::XmlReader::handleXml(graphicXmlElement, "smoothCameraTransition", generalGraphicConfig._smoothCameraTransition) &&
						!Storm::XmlReader::handleXml(graphicXmlElement, "fontSize", generalGraphicConfig._fontSize)
						)
					{
						LOG_ERROR << graphicXmlElement.first << " (inside General.Graphics) is unknown, therefore it cannot be handled";
					}
				}
			}

			if (generalGraphicConfig._wantedApplicationWidth == 0)
			{
				Storm::throwException<Storm::Exception>("screenWidth cannot be null!");
			}
			else if (generalGraphicConfig._wantedApplicationHeight == 0)
			{
				Storm::throwException<Storm::Exception>("screenHeight cannot be null!");
			}
			else if (generalGraphicConfig._fontSize <= 0)
			{
				Storm::throwException<Storm::Exception>("Font size must be a strictly positive value! Current value was " + std::to_string(generalGraphicConfig._fontSize));
			}

			/* Debug */
			Storm::GeneralDebugConfig &generalDebugConfig = _generalConfig->_generalDebugConfig;

			const auto &debugTreeOpt = generalTree.get_child_optional("Debug");
			if (debugTreeOpt.has_value())
			{
				const auto &debugTree = debugTreeOpt.value();
				for (const auto &debugXmlElement : debugTree)
				{
					if (debugXmlElement.first == "Log")
					{
						for (const auto &logXmlElement : debugXmlElement.second)
						{
							if (
								!Storm::XmlReader::handleXml(logXmlElement, "logFolderPath", generalDebugConfig._logFolderPath) &&
								!Storm::XmlReader::handleXml(logXmlElement, "logFileName", generalDebugConfig._logFileName) &&
								!Storm::XmlReader::handleXml(logXmlElement, "logLevel", generalDebugConfig._logLevel, logLevelFromString) &&
								!Storm::XmlReader::handleXml(logXmlElement, "override", generalDebugConfig._overrideLogs) &&
								!Storm::XmlReader::handleXml(logXmlElement, "fpsWatching", generalDebugConfig._shouldLogFPSWatching) &&
								!Storm::XmlReader::handleXml(logXmlElement, "logGraphicDeviceMessage", generalDebugConfig._shouldLogGraphicDeviceMessage) &&
								!Storm::XmlReader::handleXml(logXmlElement, "logPhysics", generalDebugConfig._shouldLogPhysics) &&
								!Storm::XmlReader::handleXml(logXmlElement, "removeOlderThanDays", generalDebugConfig._removeLogsOlderThanDays)
								)
							{
								LOG_ERROR << logXmlElement.first << " (inside General.Debug.Log) is unknown, therefore it cannot be handled";
							}
						}

						if (!generalDebugConfig._logFileName.empty() && generalDebugConfig._logFileName.find("$[PID]") == std::string::npos)
						{
							LOG_WARNING << "Log filename doesn't contain $[PID] macro, this could lead to unexpected behavior in case many instance of the application is run at the same time.";
						}
					}
					else if (debugXmlElement.first == "Profile")
					{
						for (const auto &profileDataXml : debugXmlElement.second)
						{
							if (
								!Storm::XmlReader::handleXml(profileDataXml, "profileSimulationSpeed", generalDebugConfig._profileSimulationSpeed)
								)
							{
								LOG_ERROR << profileDataXml.first << " (inside General.Debug.Profile) is unknown, therefore it cannot be handled";
							}
						}
					}
					else if (debugXmlElement.first == "Exception")
					{
						for (const auto &exceptionDataXml : debugXmlElement.second)
						{
							if (
								!Storm::XmlReader::handleXml(exceptionDataXml, "displayVectoredException", generalDebugConfig._displayVectoredExceptions, parseVectoredExceptionDisplayMode)
								)
							{
								LOG_ERROR << exceptionDataXml.first << " (inside General.Debug.Exception) is unknown, therefore it cannot be handled";
							}
						}
					}
					else if (debugXmlElement.first == "PhysX")
					{
						for (const auto &physXDataXml : debugXmlElement.second)
						{
							if (
								!Storm::XmlReader::handleXml(physXDataXml, "physXPvdSocket", generalDebugConfig._physXPvdDebugSocketSettings, parseSocketSettings) &&
								!Storm::XmlReader::handleXml(physXDataXml, "pvdConnectTimeout", generalDebugConfig._pvdConnectTimeoutMillisec) &&
								!Storm::XmlReader::handleXml(physXDataXml, "pvdTransmitContacts", generalDebugConfig._pvdTransmitContacts) &&
								!Storm::XmlReader::handleXml(physXDataXml, "pvdTransmitConstraints", generalDebugConfig._pvdTransmitConstraints) &&
								!Storm::XmlReader::handleXml(physXDataXml, "pvdTransmitSceneQueries", generalDebugConfig._pvdTransmitSceneQueries)
								)
							{
								LOG_ERROR << physXDataXml.first << " (inside General.Debug.PhysX) is unknown, therefore it cannot be handled";
							}
						}
					}
					else
					{
						LOG_ERROR << debugXmlElement.first << " (inside General.Debug) is unknown, therefore it cannot be handled";
					}
				}
			}

			/* Network */
			Storm::GeneralNetworkConfig &generalNetworkConfig = _generalConfig->_generalNetworkConfig;

			const auto &netTreeOpt = generalTree.get_child_optional("Network");
			if (netTreeOpt.has_value())
			{
				const auto &netTree = netTreeOpt.value();
				for (const auto &netXmlElement : netTree)
				{
					if (
						!Storm::XmlReader::handleXml(netXmlElement, "enable", generalNetworkConfig._enableNetwork) &&
						!Storm::XmlReader::handleXml(netXmlElement, "scriptSender", generalNetworkConfig._scriptSenderSocket, parseSocketSettings)
						)
					{
						LOG_ERROR << netXmlElement.first << " (inside General.Network) is unknown, therefore it cannot be handled";
					}
				}
			}

			/* Web */
			Storm::GeneralWebConfig &generalWebConfig = _generalConfig->_generalWebConfig;

			const auto &webTreeOpt = generalTree.get_child_optional("Web");
			if (webTreeOpt.has_value())
			{
				const auto &webTree = webTreeOpt.value();
				for (const auto &webXmlElement : webTree)
				{
					if (
						!Storm::XmlReader::handleXml(webXmlElement, "incognito", generalWebConfig._urlOpenIncognito) &&
						!Storm::XmlReader::handleXml(webXmlElement, "browser", generalWebConfig._preferredBrowser, parsePreferredBrowser)
						)
					{
						LOG_ERROR << webXmlElement.first << " (inside General.Web) is unknown, therefore it cannot be handled";
					}
				}
			}

			/* Simulation */
			Storm::GeneralSimulationConfig &generalSimulationConfig = _generalConfig->_generalSimulationConfig;

			const auto &simulationTreeOpt = generalTree.get_child_optional("Simulation");
			if (simulationTreeOpt.has_value())
			{
				const auto &simulationTree = simulationTreeOpt.value();
				for (const auto &simulationXmlElement : simulationTree)
				{
					if (
						!Storm::XmlReader::handleXml(simulationXmlElement, "allowNoFluid", generalSimulationConfig._allowNoFluid) &&
						!Storm::XmlReader::handleXml(simulationXmlElement, "stateRefreshFrameCount", generalSimulationConfig._stateRefreshFrameCount)
						)
					{
						LOG_ERROR << simulationXmlElement.first << " (inside General.Simulation) is unknown, therefore it cannot be handled";
					}
				}

				if (generalSimulationConfig._stateRefreshFrameCount < 0)
				{
					Storm::throwException<Storm::Exception>("State refresh frame count must be a positive value! Value was : " + std::to_string(generalSimulationConfig._stateRefreshFrameCount));
				}
				else if (generalSimulationConfig._stateRefreshFrameCount == 0)
				{
					LOG_WARNING << 
						"Simulation state refresh frame is 0. Therefore refresh system state feature will remain disabled.\n"
						"It could improve performance but we won't be advised when simulation reached a specific state.\n"
						"And we won't execute some checks and won't be advised about why this state was reached."
						;
				}
			}

			return true;
		}
	}

	return false;
}

void Storm::GeneralConfigHolder::applyMacros(const Storm::MacroConfigHolder &macroConf)
{
	Storm::GeneralDebugConfig &generalDebugConfig = _generalConfig->_generalDebugConfig;

	macroConf(generalDebugConfig._logFolderPath);
	macroConf(generalDebugConfig._logFileName);

	generalDebugConfig._logFileName = std::filesystem::path{ generalDebugConfig._logFileName }.filename().string();
}

const Storm::GeneralConfig& Storm::GeneralConfigHolder::getConfig() const
{
	return *_generalConfig;
}

Storm::GeneralConfig& Storm::GeneralConfigHolder::getConfig()
{
	return *_generalConfig;
}
