#include "SceneConfig.h"

#include "MacroConfig.h"

#include "SceneData.h"
#include "GeneralSimulationData.h"
#include "RigidBodySceneData.h"
#include "GraphicData.h"
#include "FluidData.h"
#include "BlowerData.h"
#include "ConstraintData.h"

#include "CollisionType.h"
#include "SimulationMode.h"
#include "KernelMode.h"
#include "FluidParticleLoadDenseMode.h"
#include "BlowerType.h"
#include "BlowerDef.h"

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
		if (simulModeStr == "wcsph")
		{
			return Storm::SimulationMode::WCSPH;
		}
		else if (simulModeStr == "pcisph")
		{
			return Storm::SimulationMode::PCISPH;
		}
		else
		{
			Storm::throwException<std::exception>("Simulation mode value is unknown : '" + simulModeStr + "'");
		}
	}

	Storm::KernelMode parseKernelMode(std::string kernelModeStr)
	{
		boost::algorithm::to_lower(kernelModeStr);
		if (kernelModeStr == "cubicspline")
		{
			return Storm::KernelMode::CubicSpline;
		}
		else
		{
			Storm::throwException<std::exception>("Kernel mode value is unknown : '" + kernelModeStr + "'");
		}
	}

	Storm::FluidParticleLoadDenseMode parseLoadDenseMode(std::string loadModeStr)
	{
		boost::algorithm::to_lower(loadModeStr);
		if (loadModeStr == "normal")
		{
			return Storm::FluidParticleLoadDenseMode::Normal;
		}
		else if (loadModeStr == "splishsplash")
		{
			return Storm::FluidParticleLoadDenseMode::AsSplishSplash;
		}
		else
		{
			Storm::throwException<std::exception>("Fluid particle dense load mode value is unknown : '" + loadModeStr + "'");
		}
	}

	Storm::BlowerType parseBlowerType(std::string blowerTypeStr)
	{
		boost::algorithm::to_lower(blowerTypeStr);
#define STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(BlowerTypeName, BlowerTypeXmlName, EffectAreaType, MeshMakerType) \
if (blowerTypeStr == BlowerTypeXmlName) return Storm::BlowerType::BlowerTypeName;

		STORM_XMACRO_GENERATE_BLOWERS_CODE;

#undef STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER

		Storm::throwException<std::exception>("BlowerType value is unknown : '" + blowerTypeStr + "'");
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
			!Storm::XmlReader::handleXml(generalXmlElement, "kernel", generalData._kernelMode, parseKernelMode) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "kernelCoeff", generalData._kernelCoefficient) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "CFLCoeff", generalData._cflCoeff) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "MaxCFLTime", generalData._maxCFLTime) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "physicsTime", generalData._physicsTimeInSeconds) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "fps", generalData._expectedFps) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "maxPredictIteration", generalData._maxPredictIteration) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "maxDensityError", generalData._maxDensityError) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "neighborCheckStep", generalData._recomputeNeighborhoodStep) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "simulationNoWait", generalData._simulationNoWait) &&
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
	else if (generalData._cflCoeff <= 0.f && generalData._physicsTimeInSeconds <= 0.f)
	{
		Storm::throwException<std::exception>("CFL was enabled but CFL coefficient is less or equal than 0 (value is " + std::to_string(generalData._cflCoeff) + "). It isn't allowed!");
	}
	else if (generalData._recomputeNeighborhoodStep == 0)
	{
		Storm::throwException<std::exception>("neighborCheckStep is equal to 0 which isn't allowed (we must recompute neighborhood at least one time)!");
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
			!Storm::XmlReader::handleXml(graphicXmlElement, "minColorValue", graphicData._valueForMinColor) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "maxColorValue", graphicData._valueForMaxColor) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "grid", graphicData._grid, parseVector3Element)
			)
		{
			LOG_ERROR << "tag '" << graphicXmlElement.first << "' (inside Scene.Graphic) is unknown, therefore it cannot be handled";
		}
	}

	if (graphicData._valueForMinColor > graphicData._valueForMaxColor)
	{
		Storm::throwException<std::exception>("Min color value (" + std::to_string(graphicData._valueForMinColor) + ") is greater than Max color value (" + std::to_string(graphicData._valueForMaxColor) + "). It isn't allowed!");
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
					!Storm::XmlReader::handleXml(fluidBlockDataXml, "secondPoint", fluidBlockGenerator._secondPoint, parseVector3Element) &&
					!Storm::XmlReader::handleXml(fluidBlockDataXml, "denseMode", fluidBlockGenerator._loadDenseMode, parseLoadDenseMode)
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
			!Storm::XmlReader::handleXml(fluidXmlElement, "pressureK1", fluidData._kPressureStiffnessCoeff) &&
			!Storm::XmlReader::handleXml(fluidXmlElement, "pressureK2", fluidData._kPressureExponentCoeff) &&
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
	else if (fluidData._kPressureStiffnessCoeff < 0.f)
	{
		Storm::throwException<std::exception>("Fluid " + std::to_string(fluidData._fluidId) + " pressure stiffness of " + std::to_string(fluidData._kPressureStiffnessCoeff) + " is invalid!");
	}
	else if (fluidData._kPressureExponentCoeff < 0.f)
	{
		Storm::throwException<std::exception>("Fluid " + std::to_string(fluidData._fluidId) + " pressure exponent of " + std::to_string(fluidData._kPressureExponentCoeff) + " is invalid!");
	}
	else if (fluidData._dynamicViscosity <= 0.f)
	{
		Storm::throwException<std::exception>("Fluid " + std::to_string(fluidData._fluidId) + " dynamic viscosity of " + std::to_string(fluidData._dynamicViscosity) + "N.s/m² is invalid!");
	}
	else if (fluidData._soundSpeed <= 0.f)
	{
		Storm::throwException<std::exception>("Fluid " + std::to_string(fluidData._fluidId) + " sound of speed (" + std::to_string(fluidData._dynamicViscosity) + "m/s) is invalid!");
	}

	fluidData._cinematicViscosity = fluidData._dynamicViscosity / fluidData._density;

	/* RigidBodies */
	const auto &rigidBodiesTreeOpt = srcTree.get_child_optional("RigidBodies");
	if (rigidBodiesTreeOpt.has_value())
	{
		const auto &rigidBodiesTree = rigidBodiesTreeOpt.value();

		std::vector<Storm::RigidBodySceneData> &rigidBodiesDataArray = _sceneData->_rigidBodiesData;
		rigidBodiesDataArray.reserve(rigidBodiesTree.size());

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
						!Storm::XmlReader::handleXml(rigidBodyDataXml, "wall", rbData._isWall) &&
						!Storm::XmlReader::handleXml(rigidBodyDataXml, "mass", rbData._mass) &&
						!Storm::XmlReader::handleXml(rigidBodyDataXml, "viscosity", rbData._viscosity) &&
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
					Storm::throwException<std::exception>("mass " + std::to_string(rbData._mass) + "kg is invalid (rigid body " + std::to_string(rbData._rigidBodyID) + ")!");
				}
				else if (rbData._viscosity < 0.f)
				{
					Storm::throwException<std::exception>("viscosity " + std::to_string(rbData._viscosity) + "Pa.s is invalid (rigid body " + std::to_string(rbData._rigidBodyID) + ")!");
				}
			}
			else
			{
				LOG_ERROR << rigidBodyXmlElement.first + " is not a valid tag inside 'RigidBodies'. Only 'RigidBody' is accepted!";
			}
		}
	}

	/* Contraints */
	const auto &constraintsTreeOpt = srcTree.get_child_optional("Constraints");
	if (constraintsTreeOpt.has_value())
	{
		const auto &constraintsTree = constraintsTreeOpt.value();

		std::vector<Storm::ConstraintData> &constraintsDataArray = _sceneData->_contraintsData;
		constraintsDataArray.reserve(constraintsTree.size());

		std::size_t contraintsIndex = 0;

		for (const auto &constraintXmlElement : constraintsTree)
		{
			if (constraintXmlElement.first == "Constraint")
			{
				Storm::ConstraintData &contraintData = constraintsDataArray.emplace_back();

				for (const auto &constraintDataXml : constraintXmlElement.second)
				{
					if (
						!Storm::XmlReader::handleXml(constraintDataXml, "rbId1", contraintData._rigidBodyId1) &&
						!Storm::XmlReader::handleXml(constraintDataXml, "rbId2", contraintData._rigidBodyId2)
						)
					{
						LOG_ERROR << "tag '" << constraintDataXml.first << "' (inside Scene.Contraints.Constraint) is unknown, therefore it cannot be handled";
					}
				}

				std::vector<Storm::RigidBodySceneData> &rigidBodiesDataArray = _sceneData->_rigidBodiesData;
				const auto lastToCheck = std::end(rigidBodiesDataArray);

				if (contraintData._rigidBodyId1 == std::numeric_limits<decltype(contraintData._rigidBodyId1)>::max())
				{
					Storm::throwException<std::exception>("Constraints " + std::to_string(contraintsIndex) + " rigid body 1 id wasn't set! It is a mandatory field.");
				}
				else if (contraintData._rigidBodyId2 == std::numeric_limits<decltype(contraintData._rigidBodyId2)>::max())
				{
					Storm::throwException<std::exception>("Constraints " + std::to_string(contraintsIndex) + " rigid body 2 id wasn't set! It is a mandatory field.");
				}
				else if (std::find_if(std::begin(rigidBodiesDataArray), std::end(rigidBodiesDataArray), [&contraintData](const Storm::RigidBodySceneData &registeredRb)
				{
					return registeredRb._rigidBodyID == contraintData._rigidBodyId1;
				}) == lastToCheck)
				{
					Storm::throwException<std::exception>("Constraints " + std::to_string(contraintsIndex) + " rigid body 1 id wasn't found (" + std::to_string(contraintData._rigidBodyId1) + ")!");
				}
				else if (std::find_if(std::begin(rigidBodiesDataArray), std::end(rigidBodiesDataArray), [&contraintData](const Storm::RigidBodySceneData &registeredRb)
				{
					return registeredRb._rigidBodyID == contraintData._rigidBodyId2;
				}) == lastToCheck)
				{
					Storm::throwException<std::exception>("Constraints " + std::to_string(contraintsIndex) + " rigid body 2 id wasn't found (" + std::to_string(contraintData._rigidBodyId2) + ")!");
				}
			}
			else
			{
				LOG_ERROR << constraintXmlElement.first + " is not a valid tag inside 'Constraints'. Only 'Constraint' is accepted!";
			}
		}
	}

	/* Blowers */
	const auto &blowersTreeOpt = srcTree.get_child_optional("Blowers");
	if (blowersTreeOpt.has_value())
	{
		const auto &blowersTree = blowersTreeOpt.value();

		std::vector<Storm::BlowerData> &blowersDataArray = _sceneData->_blowersData;
		blowersDataArray.reserve(blowersTree.size());

		for (const auto &blowerXmlElement : blowersTree)
		{
			if (blowerXmlElement.first == "Blower")
			{
				Storm::BlowerData &blowerData = blowersDataArray.emplace_back();
				blowerData._id = blowersDataArray.size() - 1;

				for (const auto &blowerDataXml : blowerXmlElement.second)
				{
					if (
						!Storm::XmlReader::handleXml(blowerDataXml, "type", blowerData._blowerType, parseBlowerType) &&
						!Storm::XmlReader::handleXml(blowerDataXml, "startTime", blowerData._startTimeInSeconds) &&
						!Storm::XmlReader::handleXml(blowerDataXml, "endTime", blowerData._stopTimeInSeconds) &&
						!Storm::XmlReader::handleXml(blowerDataXml, "fadeInTime", blowerData._fadeInTimeInSeconds) &&
						!Storm::XmlReader::handleXml(blowerDataXml, "fadeOutTime", blowerData._fadeOutTimeInSeconds) &&
						!Storm::XmlReader::handleXml(blowerDataXml, "radius", blowerData._radius) &&
						!Storm::XmlReader::handleXml(blowerDataXml, "dimension", blowerData._blowerDimension, parseVector3Element) &&
						!Storm::XmlReader::handleXml(blowerDataXml, "force", blowerData._blowerForce, parseVector3Element) &&
						!Storm::XmlReader::handleXml(blowerDataXml, "position", blowerData._blowerPosition, parseVector3Element)
						)
					{
						LOG_ERROR << "tag '" << blowerDataXml.first << "' (inside Scene.Blowers.Blower) is unknown, therefore it cannot be handled";
					}
				}

				if (blowerData._startTimeInSeconds < 0.f)
				{
					Storm::throwException<std::exception>("Blower " + std::to_string(blowerData._id) + " start time is invalid (it cannot be lesser or equal to 0, value was " + std::to_string(blowerData._startTimeInSeconds) + ")!");
				}
				else if (blowerData._stopTimeInSeconds == -1.f && blowerData._fadeOutTimeInSeconds > 0.f)
				{
					Storm::throwException<std::exception>("Blower " + std::to_string(blowerData._id) + " doesn't have a stop time but has a fade out time. It is illogical!");
				}
				else if (blowerData._fadeInTimeInSeconds < 0.f)
				{
					Storm::throwException<std::exception>("Blower " + std::to_string(blowerData._id) + " fade in time cannot be negative!");
				}
				else if (blowerData._fadeOutTimeInSeconds < 0.f)
				{
					Storm::throwException<std::exception>("Blower " + std::to_string(blowerData._id) + " fade out time cannot be negative!");
				}
				else if (blowerData._stopTimeInSeconds != -1.f)
				{
					if (blowerData._startTimeInSeconds >= blowerData._stopTimeInSeconds)
					{
						Storm::throwException<std::exception>(
							"Blower " + std::to_string(blowerData._id) + " end time cannot be before its start time.\n"
							"Either set it to -1 to specify that there is no stop time, or set it strictly greater than the start time!\n"
							"startTime was " + std::to_string(blowerData._startTimeInSeconds) + "s\n"
							"endTime was " + std::to_string(blowerData._stopTimeInSeconds) + "s.");
					}
					else if (blowerData._stopTimeInSeconds < blowerData._fadeOutTimeInSeconds)
					{
						Storm::throwException<std::exception>("Blower " + std::to_string(blowerData._id) + " fade out time is greater than the stop time (this means that it has faded out even before the time 0, which does not make much sense)!");
					}
					else if ((blowerData._startTimeInSeconds + blowerData._fadeInTimeInSeconds) > (blowerData._stopTimeInSeconds - blowerData._fadeOutTimeInSeconds))
					{
						Storm::throwException<std::exception>(
							"Blower " + std::to_string(blowerData._id) + " fade in and fade out overlaps... Too complex and error prone, please, change it to a way those do not overlaps!\n"
							"Fade in time start=" + std::to_string(blowerData._startTimeInSeconds) + "s; end=" + std::to_string(blowerData._startTimeInSeconds + blowerData._fadeInTimeInSeconds) + "s.\n"
							"Fade out time start=" + std::to_string(blowerData._stopTimeInSeconds - blowerData._fadeOutTimeInSeconds) + "s; end=" + std::to_string(blowerData._stopTimeInSeconds) + "s.");
					}
				}

				switch (blowerData._blowerType)
				{
				case Storm::BlowerType::Cube:
					if (blowerData._blowerDimension == Storm::Vector3::Zero())
					{
						Storm::throwException<std::exception>("Blower " + std::to_string(blowerData._id) + " (a cube) should have defined a dimension!");
					}
					else if (blowerData._blowerDimension.x() <= 0.f || blowerData._blowerDimension.y() <= 0.f || blowerData._blowerDimension.z() <= 0.f)
					{
						Storm::throwException<std::exception>("Blower " + std::to_string(blowerData._id) + " (a cube) cannot have one of its dimension value lesser or equal to 0! Specified dimension was " + Storm::toStdString(blowerData._blowerDimension));
					}
					break;

				case Storm::BlowerType::Sphere:
				case Storm::BlowerType::RepulsionSphere:
				case Storm::BlowerType::ExplosionSphere:
					if (blowerData._radius <= 0.f)
					{
						Storm::throwException<std::exception>("Blower " + std::to_string(blowerData._id) + " (a sphere) should have defined a positive non-zero radius!");
					}
					blowerData._blowerDimension = Storm::Vector3{ blowerData._radius, blowerData._radius, blowerData._radius };
					break;

				case Storm::BlowerType::PulseExplosionSphere:
					if (blowerData._radius <= 0.f)
					{
						Storm::throwException<std::exception>("Blower " + std::to_string(blowerData._id) + " (a pulse explosion) should have defined a positive non-zero radius!");
					}
					else if (blowerData._stopTimeInSeconds != -1.f)
					{
						Storm::throwException<std::exception>("Blower " + std::to_string(blowerData._id) + " (a pulse explosion) have its stop time set in stone! You cannot override it (you've set " + std::to_string(blowerData._stopTimeInSeconds) + "s).");
					}
					else if (blowerData._fadeInTimeInSeconds != 0.f)
					{
						Storm::throwException<std::exception>("Blower " + std::to_string(blowerData._id) + " (a pulse explosion) have its fadeInTime set in stone! You cannot override it (you've set " + std::to_string(blowerData._fadeInTimeInSeconds) + "s).");
					}
					else if (blowerData._fadeOutTimeInSeconds != 0.f)
					{
						Storm::throwException<std::exception>("Blower " + std::to_string(blowerData._id) + " (a pulse explosion) have its fadeInTime set in stone! You cannot override it (you've set " + std::to_string(blowerData._fadeOutTimeInSeconds) + "s).");
					}

					blowerData._blowerDimension = Storm::Vector3{ blowerData._radius, blowerData._radius, blowerData._radius };
					break;

				case Storm::BlowerType::None:
				default:
					Storm::throwException<std::exception>("Blower " + std::to_string(blowerData._id) + " should have defined a blower type, this is mandatory!");
				}
			}
			else
			{
				LOG_ERROR << blowerXmlElement.first + " is not a valid tag inside 'Blowers'. Only 'Blower' is accepted!";
			}
		}
	}
}

const Storm::SceneData& Storm::SceneConfig::getSceneData() const
{
	return *_sceneData;
}
