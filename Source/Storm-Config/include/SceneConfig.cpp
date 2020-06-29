#include "SceneConfig.h"

#include "MacroConfig.h"

#include "DataIncludes.h"

#include "CollisionType.h"
#include "SimulationMode.h"

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

	Storm::SimulationMode parseSimulationMode(std::string simulModeStr)
	{
		boost::algorithm::to_lower(simulModeStr);
		if (simulModeStr == "pcisph")
		{
			return Storm::SimulationMode::PCISPH;
		}
		else
		{
			Storm::throwException<std::exception>("Simulation mode value is unknown : '" + simulModeStr + "'");
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
			!Storm::XmlReader::handleXml(generalXmlElement, "simulation", generalData._simulationMode, parseSimulationMode) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "kernelCoeff", generalData._kernelCoefficient) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "physicsTime", generalData._physicsTimeInSeconds) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "fps", generalData._expectedFps) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "maxPredictIteration", generalData._maxPredictIteration) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "maxDensityError", generalData._maxDensityError) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "particleRadius", generalData._particleRadius)
			)
		{
			LOG_ERROR << "tag '" << generalXmlElement.first << "' (inside Scene.General) is unknown, therefore it cannot be handled";
		}
	}

	if (generalData._simulationMode == Storm::SimulationMode::None)
	{
		Storm::throwException<std::exception>("We expect to set a simulation mode, this is mandatory!");
	}
	else if (generalData._maxDensityError <= 0.f)
	{
		Storm::throwException<std::exception>("Max density error cannot be negative or equal to 0.f!");
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
			!Storm::XmlReader::handleXml(graphicXmlElement, "zFar", graphicData._zFar) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "particleDisplay", graphicData._displaySolidAsParticles) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "grid", graphicData._grid, parseVector3Element)
			)
		{
			LOG_ERROR << "tag '" << graphicXmlElement.first << "' (inside Scene.Graphic) is unknown, therefore it cannot be handled";
		}
	}


	/* Fluids */
	Storm::FluidData &fluidData = *_sceneData->_fluidData;
	const auto &fluidTree = srcTree.get_child("Fluid");
	for (const auto &fluidXmlElement : fluidTree)
	{
		if (fluidXmlElement.first == "fluidBlock")
		{
			auto &fluidBlockGenerator = fluidData._fluidGenData.emplace_back();
			for (const auto &fluidBlockDataXml : fluidXmlElement.second)
			{
				if (
					!Storm::XmlReader::handleXml(fluidBlockDataXml, "firstPoint", fluidBlockGenerator._firstPoint, parseVector3Element) &&
					!Storm::XmlReader::handleXml(fluidBlockDataXml, "secondPoint", fluidBlockGenerator._secondPoint, parseVector3Element)
					)
				{
					LOG_ERROR << "tag '" << fluidBlockDataXml.first << "' (inside Scene.Fluid.fluidBlock) is unknown, therefore it cannot be handled";
				}
			}

			if (fluidBlockGenerator._firstPoint == fluidBlockGenerator._secondPoint)
			{
				Storm::throwException<std::exception>("Generator min block value cannot be equal to the max value!");
			}
		}
		else if (
			!Storm::XmlReader::handleXml(fluidXmlElement, "id", fluidData._fluidId) &&
			!Storm::XmlReader::handleXml(fluidXmlElement, "viscosity", fluidData._dynamicViscosity) &&
			!Storm::XmlReader::handleXml(fluidXmlElement, "soundSpeed", fluidData._soundSpeed) &&
			!Storm::XmlReader::handleXml(fluidXmlElement, "density", fluidData._density)
			)
		{
			LOG_ERROR << "tag '" << fluidXmlElement.first << "' (inside Scene.Fluid) is unknown, therefore it cannot be handled";
		}
	}

	if (fluidData._fluidGenData.empty())
	{
		Storm::throwException<std::exception>("Fluid " + std::to_string(fluidData._fluidId) + " should have at least one block (an empty fluid is forbidden)!");
	}
	else if (fluidData._density <= 0.f)
	{
		Storm::throwException<std::exception>("Fluid " + std::to_string(fluidData._fluidId) + " density of " + std::to_string(fluidData._density) + "kg.m^-3 is invalid!");
	}
	else if (fluidData._dynamicViscosity <= 0.f)
	{
		Storm::throwException<std::exception>("Fluid " + std::to_string(fluidData._fluidId) + " dynamic viscosity of " + std::to_string(fluidData._dynamicViscosity) + "N.s/m² is invalid!");
	}
	else if (fluidData._soundSpeed <= 0.f)
	{
		Storm::throwException<std::exception>("Fluid " + std::to_string(fluidData._fluidId) + " sound of speed (" + std::to_string(fluidData._dynamicViscosity) + "m/s) is invalid!");
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
						!Storm::XmlReader::handleXml(rigidBodyDataXml, "mass", rbData._mass) &&
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
				else if (rbData._rigidBodyID == fluidData._fluidId)
				{
					Storm::throwException<std::exception>("RigidBody id " + std::to_string(rbData._rigidBodyID) + " is already being used by fluid data!");
				}
				else if (!std::filesystem::is_regular_file(rbData._meshFilePath))
				{
					Storm::throwException<std::exception>("'" + rbData._meshFilePath + "' is not a valid mesh file!");
				}
				else if (rbData._mass <= 0.f)
				{
					Storm::throwException<std::exception>(std::to_string(rbData._mass) + "kg is invalid (rigid body " + std::to_string(rbData._rigidBodyID) + ")!");
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
