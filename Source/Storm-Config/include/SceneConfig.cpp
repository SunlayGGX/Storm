#include "SceneConfig.h"

#include "MacroConfig.h"

#include "DataIncludes.h"

#include "ThrowException.h"
#include "XmlReader.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>


namespace
{
	Storm::Vector3 parseVector3Element(const boost::property_tree::ptree &tree)
	{
		Storm::Vector3 result;

		Storm::XmlReader::sureReadXmlAttribute(tree, result._x, "x");
		Storm::XmlReader::sureReadXmlAttribute(tree, result._y, "y");
		Storm::XmlReader::sureReadXmlAttribute(tree, result._z, "z");

		return result;
	}
}


void Storm::SceneConfig::read(const std::string &sceneConfigFilePathStr, const Storm::MacroConfig &macroConfig)
{
	_sceneData = std::make_unique<Storm::SceneData>();

	const std::filesystem::path sceneConfigFilePath{ sceneConfigFilePathStr };
	if (std::filesystem::is_regular_file(sceneConfigFilePath))
	{
		if (sceneConfigFilePath.extension() == ".xml")
		{
			boost::property_tree::ptree xmlTree;
			boost::property_tree::read_xml(sceneConfigFilePathStr, xmlTree, boost::property_tree::xml_parser::no_comments);

			const auto &generalTree = xmlTree.get_child("Scene");

			/* RigidBodies */
			const auto &rigidBodiesTreeOpt = generalTree.get_child_optional("RigidBodies");
			if (rigidBodiesTreeOpt.has_value())
			{
				const auto &rigidBodiesTree = rigidBodiesTreeOpt.value();
				auto &rigidBodiesDataArray = _sceneData->_rigidBodiesData;

				for (const auto &rigidBodyXmlElement : rigidBodiesTree)
				{
					if (rigidBodyXmlElement.first == "RigidBody")
					{
						Storm::RigidBodySceneData &rbData = rigidBodiesDataArray.emplace_back();
						for (const auto &rigidBodyDataXml : rigidBodyXmlElement.second)
						{
							if (
								!Storm::XmlReader::handleXml(rigidBodyDataXml, "id", rbData._rigidBodyID) &&
								!Storm::XmlReader::handleXml(rigidBodyDataXml, "meshFile", rbData._meshFilePath) &&
								!Storm::XmlReader::handleXml(rigidBodyDataXml, "isStatic", rbData._static) &&
								!Storm::XmlReader::handleXml(rigidBodyDataXml, "translation", rbData._translation, parseVector3Element) &&
								!Storm::XmlReader::handleXml(rigidBodyDataXml, "rotation", rbData._rotation, parseVector3Element) &&
								!Storm::XmlReader::handleXml(rigidBodyDataXml, "scale", rbData._scale, parseVector3Element)
								)
							{
								LOG_ERROR << rigidBodyDataXml.first << " (inside Scene.Rigidbodies.RigidBody) is unknown, therefore it cannot be handled";
							}
						}

						macroConfig(rbData._meshFilePath);

						// Minus 1 because of course, the rbData that we are currently filling has the same id than itself... 
						const auto lastToCheck = std::end(rigidBodiesDataArray) - 1;
						if (const auto found = std::find_if(std::begin(rigidBodiesDataArray), lastToCheck, [&rbData](const Storm::RigidBodySceneData &registeredRb)
						{
							return registeredRb._rigidBodyID == rbData._rigidBodyID;
						}); found != lastToCheck)
						{
							Storm::throwException<std::exception>("RigidBody id " + std::to_string(rbData._rigidBodyID) + " is already used!");
						}
						else if (!std::filesystem::is_regular_file(rbData._meshFilePath))
						{
							Storm::throwException<std::exception>("'" + rbData._meshFilePath + "' is not a valid mesh file!");
						}
					}
					else
					{
						LOG_ERROR << rigidBodyXmlElement.first + " is not a valid tag inside 'RigidBodies'. Only 'RigidBody' is accepted!";
					}
				}
			}

			return true;
		}
	}

	return false;
}

const Storm::SceneData& Storm::SceneConfig::getSceneData() const
{
	return *_sceneData;
}
