#include "SceneConfig.h"

#include "MacroConfig.h"

#include "DataIncludes.h"

#include "CollisionType.h"

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

		Storm::XmlReader::sureReadXmlAttribute(tree, result.x(), "x");
		Storm::XmlReader::sureReadXmlAttribute(tree, result.y(), "y");
		Storm::XmlReader::sureReadXmlAttribute(tree, result.z(), "z");

		return result;
	}

	Storm::CollisionType parseCollisionType(std::string collisionTypeStr)
	{
		boost::algorithm::to_lower(collisionTypeStr);
		if (collisionTypeStr == "cube")
		{
			return Storm::CollisionType::Cube;
		}
		else if (collisionTypeStr == "sphere")
		{
			return Storm::CollisionType::Sphere;
		}
		else if (collisionTypeStr == "none")
		{
			return Storm::CollisionType::None;
		}
		else
		{
			Storm::throwException<std::exception>("CollisionType value in rigidbody is unknown : '" + collisionTypeStr + "'");
		}
	}
}


void Storm::SceneConfig::read(const std::string &sceneConfigFilePathStr, const Storm::MacroConfig &macroConfig)
{
	_sceneData = std::make_unique<Storm::SceneData>();

	boost::property_tree::ptree xmlTree;
	boost::property_tree::read_xml(sceneConfigFilePathStr, xmlTree, boost::property_tree::xml_parser::no_comments);

	const auto &srcTree = xmlTree.get_child("Scene");


	/* General */
	// This is mandatory, so no optional
	Storm::GeneralSimulationData &generalData = *_sceneData->_generalSimulationData;
	const auto &generalTree = srcTree.get_child("General");
	for (const auto &generalXmlElement : generalTree)
	{
		if (
			!Storm::XmlReader::handleXml(generalXmlElement, "startPaused", generalData._startPaused) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "gravity", generalData._gravity, parseVector3Element) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "kernelCoeff", generalData._kernelCoefficient) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "particleRadius", generalData._particleRadius)
			)
		{
			LOG_ERROR << "tag '" << generalXmlElement.first << "' (inside Scene.General) is unknown, therefore it cannot be handled";
		}
	}


	/* Graphic */
	Storm::GraphicData &graphicData = *_sceneData->_graphicData;
	const auto &graphicTree = srcTree.get_child("Graphic");
	for (const auto &graphicXmlElement : graphicTree)
	{
		if (
			!Storm::XmlReader::handleXml(graphicXmlElement, "cameraPosition", graphicData._cameraPosition, parseVector3Element) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "cameraLookAt", graphicData._cameraLookAt, parseVector3Element) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "zNear", graphicData._zNear) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "zFar", graphicData._zFar)
			)
		{
			LOG_ERROR << "tag '" << graphicXmlElement.first << "' (inside Scene.Graphic) is unknown, therefore it cannot be handled";
		}
	}


	/* RigidBodies */
	const auto &rigidBodiesTreeOpt = srcTree.get_child_optional("RigidBodies");
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
						!Storm::XmlReader::handleXml(rigidBodyDataXml, "staticFrictionCoeff", rbData._staticFrictionCoefficient) &&
						!Storm::XmlReader::handleXml(rigidBodyDataXml, "dynamicFrictionCoeff", rbData._dynamicFrictionCoefficient) &&
						!Storm::XmlReader::handleXml(rigidBodyDataXml, "restitutionCoeff", rbData._restitutionCoefficient) &&
						!Storm::XmlReader::handleXml(rigidBodyDataXml, "isStatic", rbData._static) &&
						!Storm::XmlReader::handleXml(rigidBodyDataXml, "collisionType", rbData._collisionShape, parseCollisionType) &&
						!Storm::XmlReader::handleXml(rigidBodyDataXml, "translation", rbData._translation, parseVector3Element) &&
						!Storm::XmlReader::handleXml(rigidBodyDataXml, "rotation", rbData._rotation, parseVector3Element) &&
						!Storm::XmlReader::handleXml(rigidBodyDataXml, "scale", rbData._scale, parseVector3Element)
						)
					{
						LOG_ERROR << "tag '" << rigidBodyDataXml.first << "' (inside Scene.Rigidbodies.RigidBody) is unknown, therefore it cannot be handled";
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
}

const Storm::SceneData& Storm::SceneConfig::getSceneData() const
{
	return *_sceneData;
}
