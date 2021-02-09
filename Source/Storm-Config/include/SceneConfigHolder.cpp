#include "SceneConfigHolder.h"

#include "MacroConfigHolder.h"
#include "GeneralConfigHolder.h"

#include "SceneConfig.h"
#include "SceneSimulationConfig.h"
#include "SceneRigidBodyConfig.h"
#include "SceneGraphicConfig.h"
#include "SceneFluidConfig.h"
#include "SceneBlowerConfig.h"
#include "SceneConstraintConfig.h"
#include "SceneRecordConfig.h"
#include "SceneScriptConfig.h"
#include "SceneFluidCustomDFSPHConfig.h"

#include "GeneralConfig.h"

#include "CollisionType.h"
#include "SimulationMode.h"
#include "KernelMode.h"
#include "FluidParticleLoadDenseMode.h"
#include "BlowerType.h"
#include "BlowerDef.h"
#include "InsideParticleRemovalTechnique.h"
#include "LayeringGenerationTechnique.h"
#include "ViscosityMethod.h"

#include "ColorChecker.h"
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

	Storm::Vector4 parseColor4Element(const boost::property_tree::ptree &tree)
	{
		Storm::Vector4 result;

		Storm::XmlReader::sureReadXmlAttribute(tree, result.x(), "r");
		Storm::XmlReader::sureReadXmlAttribute(tree, result.y(), "g");
		Storm::XmlReader::sureReadXmlAttribute(tree, result.z(), "b");
		Storm::XmlReader::sureReadXmlAttribute(tree, result.w(), "a");

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
		else if (collisionTypeStr == "custom")
		{
			return Storm::CollisionType::Custom;
		}
		else if (collisionTypeStr == "particle")
		{
			return Storm::CollisionType::IndividualParticle;
		}
		else if (collisionTypeStr == "none")
		{
			return Storm::CollisionType::None;
		}
		else
		{
			Storm::throwException<Storm::Exception>("CollisionType value in rigidbody is unknown : '" + collisionTypeStr + "'");
		}
	}

	Storm::SimulationMode parseSimulationMode(std::string &inOutSimulModeStr)
	{
		boost::algorithm::to_upper(inOutSimulModeStr);
		if (inOutSimulModeStr == "WCSPH")
		{
			return Storm::SimulationMode::WCSPH;
		}
		else if (inOutSimulModeStr == "PCISPH")
		{
			return Storm::SimulationMode::PCISPH;
		}
		else if (inOutSimulModeStr == "IISPH")
		{
			return Storm::SimulationMode::IISPH;
		}
		else if (inOutSimulModeStr == "DFSPH")
		{
			return Storm::SimulationMode::DFSPH;
		}
		else
		{
			Storm::throwException<Storm::Exception>("Simulation mode value is unknown : '" + inOutSimulModeStr + "'");
		}
	}

	Storm::KernelMode parseKernelMode(std::string kernelModeStr)
	{
		boost::algorithm::to_lower(kernelModeStr);
		if (kernelModeStr == "cubicspline")
		{
			return Storm::KernelMode::CubicSpline;
		}
		else if (kernelModeStr == "splishsplashcubicspline")
		{
			return Storm::KernelMode::SplishSplashCubicSpline;
		}
		else
		{
			Storm::throwException<Storm::Exception>("Kernel mode value is unknown : '" + kernelModeStr + "'");
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
			Storm::throwException<Storm::Exception>("Fluid particle dense load mode value is unknown : '" + loadModeStr + "'");
		}
	}

	Storm::BlowerType parseBlowerType(std::string blowerTypeStr)
	{
		boost::algorithm::to_lower(blowerTypeStr);
#define STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(BlowerTypeName, BlowerTypeXmlName, EffectAreaType, MeshMakerType) \
if (blowerTypeStr == BlowerTypeXmlName) return Storm::BlowerType::BlowerTypeName;

		STORM_XMACRO_GENERATE_BLOWERS_CODE;

#undef STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER

		Storm::throwException<Storm::Exception>("BlowerType value is unknown : '" + blowerTypeStr + "'");
	}

	Storm::InsideParticleRemovalTechnique parseInsideRemovalTech(std::string techTypeStr)
	{
		boost::algorithm::to_lower(techTypeStr);
		if (techTypeStr == "none")
		{
			return Storm::InsideParticleRemovalTechnique::None;
		}
		else if (techTypeStr == "normals")
		{
			return Storm::InsideParticleRemovalTechnique::Normals;
		}
		else
		{
			Storm::throwException<Storm::Exception>("Fluid particle removal technique value is unknown : '" + techTypeStr + "'");
		}
	}

	Storm::LayeringGenerationTechnique parseLayeringGenerationTechnique(std::string layeringTechTypeStr)
	{
		boost::algorithm::to_lower(layeringTechTypeStr);
		if (layeringTechTypeStr == "scaling")
		{
			return Storm::LayeringGenerationTechnique::Scaling;
		}
		else if (layeringTechTypeStr == "dissociated")
		{
			return Storm::LayeringGenerationTechnique::DissociatedTriangle;
		}
		else
		{
			Storm::throwException<Storm::Exception>("Layering generation technique value is unknown : '" + layeringTechTypeStr + "'");
		}
	}

	Storm::ViscosityMethod parseViscosityMethod(std::string viscosityMethodStr)
	{
		boost::algorithm::to_lower(viscosityMethodStr);
		if (viscosityMethodStr == "xsph")
		{
			return Storm::ViscosityMethod::XSPH;
		}
		else if (viscosityMethodStr == "standard")
		{
			return Storm::ViscosityMethod::Standard;
		}
		else
		{
			Storm::throwException<Storm::Exception>("Viscosity method value is unknown : '" + viscosityMethodStr + "'");
		}
	}

	class AnimationsKeeper
	{
	public:
		const boost::property_tree::ptree& retrieveAnimationXml(const std::string &animationPath)
		{
			if (const auto found = _xmlTree.find(animationPath); found != std::end(_xmlTree))
			{
				return found->second;
			}
			else
			{
				boost::property_tree::ptree animationXmlTree;

				boost::property_tree::read_xml(animationPath, animationXmlTree, boost::property_tree::xml_parser::no_comments);
				return _xmlTree.emplace_hint(found, animationPath, std::move(animationXmlTree))->second;
			}
		}

	private:
		std::map<std::string, boost::property_tree::ptree> _xmlTree;
	};

	struct XmlAnimationParserPolicy
	{
	public:
		template<class Policy, class ValueType>
		static std::string parse(const ValueType &xml)
		{
			std::stringstream str;
			boost::property_tree::xml_parser::write_xml_element(str, "Animation", xml, 0, boost::property_tree::xml_writer_make_settings<std::string>());
			return str.str();
		}
	};
}


void Storm::SceneConfigHolder::read(const std::string &sceneConfigFilePathStr, const Storm::MacroConfigHolder &macroConfig, const Storm::GeneralConfigHolder &generalConfigHolder)
{
	_sceneConfig = std::make_unique<Storm::SceneConfig>();

	boost::property_tree::ptree xmlTree;
	boost::property_tree::read_xml(sceneConfigFilePathStr, xmlTree, boost::property_tree::xml_parser::no_comments);

	const auto &srcTree = xmlTree.get_child("Scene");

	/* General */
	// This is mandatory, so no optional
	Storm::SceneSimulationConfig &sceneSimulationConfig = _sceneConfig->_simulationConfig;

	const auto &generalTree = srcTree.get_child("General");
	for (const auto &generalXmlElement : generalTree)
	{
		if (
			!Storm::XmlReader::handleXml(generalXmlElement, "startPaused", sceneSimulationConfig._startPaused) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "gravity", sceneSimulationConfig._gravity, parseVector3Element) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "simulation", sceneSimulationConfig._simulationModeStr) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "kernel", sceneSimulationConfig._kernelMode, parseKernelMode) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "kernelCoeff", sceneSimulationConfig._kernelCoefficient) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "maxKernelIncrementCoeff", sceneSimulationConfig._maxKernelIncrementCoeff) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "kernelIncrementCoeffEndTime", sceneSimulationConfig._kernelIncrementSpeedInSeconds) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "fluidViscosityMethod", sceneSimulationConfig._fluidViscoMethod, parseViscosityMethod) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "rbViscosityMethod", sceneSimulationConfig._rbViscoMethod, parseViscosityMethod) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "CFLCoeff", sceneSimulationConfig._cflCoeff) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "MaxCFLTime", sceneSimulationConfig._maxCFLTime) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "CFLIteration", sceneSimulationConfig._maxCFLIteration) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "physicsTime", sceneSimulationConfig._physicsTimeInSec) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "fps", sceneSimulationConfig._expectedFps) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "minPredictIteration", sceneSimulationConfig._minPredictIteration) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "maxPredictIteration", sceneSimulationConfig._maxPredictIteration) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "maxDensityError", sceneSimulationConfig._maxDensityError) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "neighborCheckStep", sceneSimulationConfig._recomputeNeighborhoodStep) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "simulationNoWait", sceneSimulationConfig._simulationNoWait) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "particleRadius", sceneSimulationConfig._particleRadius) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "endPhysicsTime", sceneSimulationConfig._endSimulationPhysicsTimeInSeconds) &&
			!Storm::XmlReader::handleXml(generalXmlElement, "startFixRigidBodies", sceneSimulationConfig._fixRigidBodyAtStartTime)
			)
		{
			LOG_ERROR << "tag '" << generalXmlElement.first << "' (inside Scene.General) is unknown, therefore it cannot be handled";
		}
	}

	if (!sceneSimulationConfig._simulationModeStr.empty())
	{
		sceneSimulationConfig._simulationMode = parseSimulationMode(sceneSimulationConfig._simulationModeStr);
	}

	if (sceneSimulationConfig._simulationMode == Storm::SimulationMode::None)
	{
		Storm::throwException<Storm::Exception>("We expect to set a simulation mode, this is mandatory!");
	}
	else if (sceneSimulationConfig._maxDensityError <= 0.f)
	{
		Storm::throwException<Storm::Exception>("Max density error cannot be negative or equal to 0.f!");
	}
	else if (sceneSimulationConfig._minPredictIteration > sceneSimulationConfig._maxPredictIteration)
	{
		Storm::throwException<Storm::Exception>("Max prediction iteration (" + std::to_string(sceneSimulationConfig._maxPredictIteration) + ") should be greater or equal than min prediction iter (" + std::to_string(sceneSimulationConfig._minPredictIteration) + ")!");
	}
	else if (sceneSimulationConfig._maxPredictIteration == 0)
	{
		Storm::throwException<Storm::Exception>("Max prediction iteration shouldn't be equal to 0 (we should at least compute the iteration one time!");
	}
	else if (sceneSimulationConfig._physicsTimeInSec <= 0.f)
	{
		if (sceneSimulationConfig._cflCoeff <= 0.f)
		{
			Storm::throwException<Storm::Exception>("CFL was enabled but CFL coefficient is less or equal than 0 (value is " + std::to_string(sceneSimulationConfig._cflCoeff) + "). It isn't allowed!");
		}
		else if (sceneSimulationConfig._maxCFLIteration <= 0)
		{
			Storm::throwException<Storm::Exception>("CFL iteration should be greater or equal than 1 (value is " + std::to_string(sceneSimulationConfig._maxCFLIteration) + ").");
		}
	}
	if (sceneSimulationConfig._recomputeNeighborhoodStep == 0)
	{
		Storm::throwException<Storm::Exception>("neighborCheckStep is equal to 0 which isn't allowed (we must recompute neighborhood at least one time)!");
	}
	else if (sceneSimulationConfig._endSimulationPhysicsTimeInSeconds != -1.f && sceneSimulationConfig._endSimulationPhysicsTimeInSeconds <= 0.f)
	{
		Storm::throwException<Storm::Exception>("end simulation time was set to a negative or zero time (" + std::to_string(sceneSimulationConfig._endSimulationPhysicsTimeInSeconds) + "). It isn't allowed!");
	}
	else if (sceneSimulationConfig._kernelCoefficient < 0.f)
	{
		Storm::throwException<Storm::Exception>("Kernel coefficient value shouldn't be below 0 (" + std::to_string(sceneSimulationConfig._kernelCoefficient) + ")");
	}
	else if (sceneSimulationConfig._maxKernelIncrementCoeff <= -sceneSimulationConfig._kernelCoefficient)
	{
		Storm::throwException<Storm::Exception>("We cannot decrease the kernel coefficient to a value below or equal to 0:\n"
			"kernel coeff : " + std::to_string(sceneSimulationConfig._kernelCoefficient) + "\n"
			"Requested decrease : " + std::to_string(sceneSimulationConfig._maxKernelIncrementCoeff));
	}
	else if (sceneSimulationConfig._kernelIncrementSpeedInSeconds != -1.f && sceneSimulationConfig._kernelIncrementSpeedInSeconds < 0.f)
	{
		Storm::throwException<Storm::Exception>("Time to finish increasing the kernel coefficient in seconds should be positive or -1 (disabled). Value was " + std::to_string(sceneSimulationConfig._kernelIncrementSpeedInSeconds));
	}

	if (sceneSimulationConfig._kernelIncrementSpeedInSeconds != -1.f && sceneSimulationConfig._maxKernelIncrementCoeff == 0.f)
	{
		LOG_WARNING << "We enabled the runtime kernel increment feature but didn't set a max increment coeff. The feature will be disabled.";
	}
	else if (sceneSimulationConfig._kernelIncrementSpeedInSeconds == -1.f && sceneSimulationConfig._maxKernelIncrementCoeff != 0.f)
	{
		LOG_WARNING << "We disabled the runtime kernel increment feature because we haven't set a end time for the increment while setting a end value.";
	}

	sceneSimulationConfig._computeCFL = sceneSimulationConfig._physicsTimeInSec <= 0.f;

	/* Graphic */
	const float defaultLineThickness = sceneSimulationConfig._particleRadius / 3.f;

	Storm::SceneGraphicConfig &graphicConfig = _sceneConfig->_graphicConfig;
	graphicConfig._constraintThickness = defaultLineThickness;
	graphicConfig._forceThickness = defaultLineThickness;

	const auto &graphicTree = srcTree.get_child("Graphic");
	for (const auto &graphicXmlElement : graphicTree)
	{
		if (
			!Storm::XmlReader::handleXml(graphicXmlElement, "cameraPosition", graphicConfig._cameraPosition, parseVector3Element) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "cameraLookAt", graphicConfig._cameraLookAt, parseVector3Element) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "zNear", graphicConfig._zNear) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "zFar", graphicConfig._zFar) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "particleDisplay", graphicConfig._displaySolidAsParticles) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "minVelocityColorValue", graphicConfig._velocityNormMinColor) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "maxVelocityColorValue", graphicConfig._velocityNormMaxColor) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "minPressureColorValue", graphicConfig._pressureMinColor) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "maxPressureColorValue", graphicConfig._pressureMaxColor) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "minDensityColorValue", graphicConfig._densityMinColor) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "maxDensityColorValue", graphicConfig._densityMaxColor) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "constraintThickness", graphicConfig._constraintThickness) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "constraintColor", graphicConfig._constraintColor, parseColor4Element) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "forceThickness", graphicConfig._forceThickness) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "forceColor", graphicConfig._forceColor, parseColor4Element) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "blowerAlpha", graphicConfig._blowerAlpha) &&
			!Storm::XmlReader::handleXml(graphicXmlElement, "grid", graphicConfig._grid, parseVector3Element)
			)
		{
			LOG_ERROR << "tag '" << graphicXmlElement.first << "' (inside Scene.Graphic) is unknown, therefore it cannot be handled";
		}
	}

	if (graphicConfig._velocityNormMinColor > graphicConfig._velocityNormMaxColor)
	{
		Storm::throwException<Storm::Exception>("Min velocity color value (" + std::to_string(graphicConfig._velocityNormMinColor) + ") is greater than Max color value (" + std::to_string(graphicConfig._velocityNormMaxColor) + "). It isn't allowed!");
	}
	if (graphicConfig._pressureMinColor > graphicConfig._pressureMaxColor)
	{
		Storm::throwException<Storm::Exception>("Min pressure color value (" + std::to_string(graphicConfig._pressureMinColor) + ") is greater than Max color value (" + std::to_string(graphicConfig._pressureMaxColor) + "). It isn't allowed!");
	}
	if (graphicConfig._densityMinColor > graphicConfig._densityMaxColor)
	{
		Storm::throwException<Storm::Exception>("Min velocity color value (" + std::to_string(graphicConfig._densityMinColor) + ") is greater than Max color value (" + std::to_string(graphicConfig._densityMaxColor) + "). It isn't allowed!");
	}
	else if (Storm::ColorCheckerHelper::channelIsInvalid(graphicConfig._blowerAlpha))
	{
		Storm::throwException<Storm::Exception>("blower alpha channel value is invalid (" + std::to_string(graphicConfig._blowerAlpha) + "). It should be in the range 0.0 and 1.0 included!");
	}
	else if (Storm::ColorCheckerHelper::isInvalid(graphicConfig._constraintColor))
	{
		Storm::throwException<Storm::Exception>("Constraint color is invalid (" + Storm::toStdString(graphicConfig._constraintColor) + ")! Each channel must be between 0.0 and 1.0 included!");
	}
	else if (graphicConfig._constraintThickness <= 0.f)
	{
		Storm::throwException<Storm::Exception>("Constraint thickness is invalid (" + Storm::toStdString(graphicConfig._constraintThickness) + ")! It must be a positive non zero value.");
	}
	else if (Storm::ColorCheckerHelper::isInvalid(graphicConfig._forceColor))
	{
		Storm::throwException<Storm::Exception>("Selected particle force color is invalid (" + Storm::toStdString(graphicConfig._forceColor) + ")! Each channel must be between 0.0 and 1.0 included!");
	}
	else if (graphicConfig._forceThickness <= 0.f)
	{
		Storm::throwException<Storm::Exception>("Selected particle force thickness is invalid (" + Storm::toStdString(graphicConfig._forceThickness) + ")! It must be a positive non zero value.");
	}

	/* Record */
	const auto &recordTreeOpt = srcTree.get_child_optional("Record");
	if (recordTreeOpt.has_value())
	{
		Storm::SceneRecordConfig &recordConfig = _sceneConfig->_recordConfig;

		const auto &recordTree = recordTreeOpt.value();
		for (const auto &recordXmlElement : recordTree)
		{
			if (
				!Storm::XmlReader::handleXml(recordXmlElement, "recordFps", recordConfig._recordFps) &&
				!Storm::XmlReader::handleXml(recordXmlElement, "recordFile", recordConfig._recordFilePath) &&
				!Storm::XmlReader::handleXml(recordXmlElement, "replayRealTime", recordConfig._replayRealTime)
				)
			{
				LOG_ERROR << "tag '" << recordXmlElement.first << "' (inside Scene.Record) is unknown, therefore it cannot be handled";
			}
		}

		if (recordConfig._recordFps <= 0.f && recordConfig._recordFps != -1.f)
		{
			Storm::throwException<Storm::Exception>("Record fps should remain unset or should be a positive value (currently " + std::to_string(recordConfig._recordFps) + ")");
		}

		// The other validations will be done later.
	}

	/* Script */
	Storm::SceneScriptConfig &scriptConfig = _sceneConfig->_scriptConfig;

	const auto &scriptTreeOpt = srcTree.get_child_optional("Script");
	if (scriptTreeOpt.has_value())
	{
		const auto &scriptTree = scriptTreeOpt.value();
		for (const auto &scriptXmlElement : scriptTree)
		{
			if (scriptXmlElement.first == "Init")
			{
				auto &scriptInitConfigData = scriptConfig._initScriptFiles;
				for (const auto &scriptInitDataXml : scriptXmlElement.second)
				{
					if (
						!Storm::XmlReader::handleXml(scriptInitDataXml, "initScriptFile", scriptInitConfigData._filePath)
						)
					{
						LOG_ERROR << "tag '" << scriptInitDataXml.first << "' (inside Scene.Script.Init) is unknown, therefore it cannot be handled";
					}
				}
			}
			else if (scriptXmlElement.first == "Runtime")
			{
				auto &scriptRuntimeConfigData = scriptConfig._scriptFilePipe;
				for (const auto &scriptRuntimeDataXml : scriptXmlElement.second)
				{
					if (
						!Storm::XmlReader::handleXml(scriptRuntimeDataXml, "watchedScriptFile", scriptRuntimeConfigData._filePath) &&
						!Storm::XmlReader::handleXml(scriptRuntimeDataXml, "refreshTime", scriptRuntimeConfigData._refreshRateInMillisec)
						)
					{
						LOG_ERROR << "tag '" << scriptRuntimeDataXml.first << "' (inside Scene.Script.Runtime) is unknown, therefore it cannot be handled";
					}
				}

				if (scriptRuntimeConfigData._refreshRateInMillisec == 0)
				{
					Storm::throwException<Storm::Exception>("Watched script file refresh rate cannot be equal to 0! Specify a non null positive value.");
				}
			}
			else if (
				!Storm::XmlReader::handleXml(scriptXmlElement, "enabled", scriptConfig._enabled)
				)
			{
				LOG_ERROR << "tag '" << scriptXmlElement.first << "' (inside Scene.Script) is unknown, therefore it cannot be handled";
			}
		}
	}

	macroConfig(scriptConfig._initScriptFiles._filePath);
	macroConfig(scriptConfig._scriptFilePipe._filePath);

	/* Fluids */
	Storm::SceneFluidConfig &fluidConfig = _sceneConfig->_fluidConfig;

	switch (sceneSimulationConfig._simulationMode)
	{
	case Storm::SimulationMode::DFSPH:
		fluidConfig._customSimulationSettings = std::make_unique<Storm::SceneFluidCustomDFSPHConfig>();
		break;

	case Storm::SimulationMode::WCSPH:
	case Storm::SimulationMode::PCISPH:
	case Storm::SimulationMode::IISPH:
	default:
		fluidConfig._customSimulationSettings = std::make_unique<Storm::SceneFluidDefaultCustomConfig>();
		break;
	}

	const auto &fluidTreeOpt = srcTree.get_child_optional("Fluid");
	if (fluidTreeOpt.has_value())
	{
		const auto &fluidTree = fluidTreeOpt.value();
		for (const auto &fluidXmlElement : fluidTree)
		{
			if (fluidXmlElement.first == "fluidBlock")
			{
				auto &fluidBlockGenerator = fluidConfig._fluidGenConfig.emplace_back();
				for (const auto &fluidBlockConfigXml : fluidXmlElement.second)
				{
					if (
						!Storm::XmlReader::handleXml(fluidBlockConfigXml, "firstPoint", fluidBlockGenerator._firstPoint, parseVector3Element) &&
						!Storm::XmlReader::handleXml(fluidBlockConfigXml, "secondPoint", fluidBlockGenerator._secondPoint, parseVector3Element) &&
						!Storm::XmlReader::handleXml(fluidBlockConfigXml, "denseMode", fluidBlockGenerator._loadDenseMode, parseLoadDenseMode)
						)
					{
						LOG_ERROR << "tag '" << fluidBlockConfigXml.first << "' (inside Scene.Fluid.fluidBlock) is unknown, therefore it cannot be handled";
					}
				}

				if (fluidBlockGenerator._firstPoint == fluidBlockGenerator._secondPoint)
				{
					Storm::throwException<Storm::Exception>("Generator min block value cannot be equal to the max value!");
				}
			}
			else if (fluidXmlElement.first == "UnitParticles")
			{
				for (const auto &fluidParticleGenConfigXml : fluidXmlElement.second)
				{
					auto &fluidParticleGenerator = fluidConfig._fluidUnitParticleGenConfig.emplace_back();
					if (!Storm::XmlReader::handleXml(fluidParticleGenConfigXml, "position", fluidParticleGenerator._position, parseVector3Element))
					{
						LOG_ERROR << "tag '" << fluidParticleGenConfigXml.first << "' (inside Scene.Fluid.UnitParticles) is unknown, therefore it cannot be handled";
					}
				}
			}
			else if (sceneSimulationConfig._simulationMode == SimulationMode::DFSPH && fluidXmlElement.first == "DFSPH")
			{
				Storm::SceneFluidCustomDFSPHConfig &fluidDfsphConfig = static_cast<Storm::SceneFluidCustomDFSPHConfig &>(*fluidConfig._customSimulationSettings);
				for (const auto &fluidParticleCustomSimulConfigXml : fluidXmlElement.second)
				{
					if (
						!Storm::XmlReader::handleXml(fluidParticleCustomSimulConfigXml, "pressurePredictKCoeff", fluidDfsphConfig._kPressurePredictedCoeff) &&
						!Storm::XmlReader::handleXml(fluidParticleCustomSimulConfigXml, "enableThresholdDensity", fluidDfsphConfig._enableThresholdDensity) &&
						!Storm::XmlReader::handleXml(fluidParticleCustomSimulConfigXml, "neighborThresholdDensity", fluidDfsphConfig._neighborThresholdDensity)
						)
					{
						LOG_ERROR << "tag '" << fluidParticleCustomSimulConfigXml.first << "' (inside Scene.Fluid.DFSPH) is unknown, therefore it cannot be handled";
					}
				}

				if (fluidDfsphConfig._kPressurePredictedCoeff < 0.f)
				{
					Storm::throwException<Storm::Exception>("Fluid " + std::to_string(fluidConfig._fluidId) + " predicted pressure coefficient of " + std::to_string(fluidDfsphConfig._kPressurePredictedCoeff) + " is invalid!");
				}
				else if (fluidDfsphConfig._enableThresholdDensity && fluidDfsphConfig._neighborThresholdDensity == 0)
				{
					Storm::throwException<Storm::Exception>("Fluid " + std::to_string(fluidConfig._fluidId) + " has enabled DFSPH density threshold but has an invalid threshold of 0!");
				}
			}
			else if (
				!Storm::XmlReader::handleXml(fluidXmlElement, "id", fluidConfig._fluidId) &&
				!Storm::XmlReader::handleXml(fluidXmlElement, "viscosity", fluidConfig._dynamicViscosity) &&
				!Storm::XmlReader::handleXml(fluidXmlElement, "soundSpeed", fluidConfig._soundSpeed) &&
				!Storm::XmlReader::handleXml(fluidXmlElement, "pressureK1", fluidConfig._kPressureStiffnessCoeff) &&
				!Storm::XmlReader::handleXml(fluidXmlElement, "pressureK2", fluidConfig._kPressureExponentCoeff) &&
				!Storm::XmlReader::handleXml(fluidXmlElement, "relaxationCoeff", fluidConfig._relaxationCoefficient) &&
				!Storm::XmlReader::handleXml(fluidXmlElement, "initRelaxationCoeff", fluidConfig._pressureInitRelaxationCoefficient) &&
				!Storm::XmlReader::handleXml(fluidXmlElement, "enableGravity", fluidConfig._gravityEnabled) &&
				!Storm::XmlReader::handleXml(fluidXmlElement, "removeCollidingParticles", fluidConfig._removeParticlesCollidingWithRb) &&
				!Storm::XmlReader::handleXml(fluidXmlElement, "density", fluidConfig._density)
				)
			{
				LOG_ERROR << "tag '" << fluidXmlElement.first << "' (inside Scene.Fluid) is unknown, therefore it cannot be handled";
			}
		}

		if (fluidConfig._fluidId == std::numeric_limits<decltype(fluidConfig._fluidId)>::max())
		{
			Storm::throwException<Storm::Exception>("Fluid id should be set using 'id' tag!");
		}
		else if (fluidConfig._fluidGenConfig.empty() && fluidConfig._fluidUnitParticleGenConfig.empty())
		{
			Storm::throwException<Storm::Exception>("Fluid " + std::to_string(fluidConfig._fluidId) + " should have at least one block (an empty fluid is forbidden)!");
		}
		else if (fluidConfig._density <= 0.f)
		{
			Storm::throwException<Storm::Exception>("Fluid " + std::to_string(fluidConfig._fluidId) + " density of " + std::to_string(fluidConfig._density) + "kg.m^-3 is invalid!");
		}
		else if (fluidConfig._kPressureStiffnessCoeff < 0.f)
		{
			Storm::throwException<Storm::Exception>("Fluid " + std::to_string(fluidConfig._fluidId) + " pressure stiffness of " + std::to_string(fluidConfig._kPressureStiffnessCoeff) + " is invalid!");
		}
		else if (fluidConfig._kPressureExponentCoeff < 0.f)
		{
			Storm::throwException<Storm::Exception>("Fluid " + std::to_string(fluidConfig._fluidId) + " pressure exponent of " + std::to_string(fluidConfig._kPressureExponentCoeff) + " is invalid!");
		}
		else if (fluidConfig._relaxationCoefficient < 0.f || fluidConfig._relaxationCoefficient > 1.f)
		{
			Storm::throwException<Storm::Exception>("Fluid " + std::to_string(fluidConfig._fluidId) + " relaxation coefficient " + std::to_string(fluidConfig._relaxationCoefficient) + " is invalid (it should be between 0.0 and 1.0 included)!");
		}
		else if (fluidConfig._pressureInitRelaxationCoefficient < 0.f)
		{
			Storm::throwException<Storm::Exception>("Fluid " + std::to_string(fluidConfig._fluidId) + " init relaxation coefficient " + std::to_string(fluidConfig._pressureInitRelaxationCoefficient) + " is invalid (it should be greater or equal to 0.0)!");
		}
		else if (fluidConfig._dynamicViscosity <= 0.f)
		{
			Storm::throwException<Storm::Exception>("Fluid " + std::to_string(fluidConfig._fluidId) + " dynamic viscosity of " + std::to_string(fluidConfig._dynamicViscosity) + "N.s/m² is invalid!");
		}
		else if (fluidConfig._soundSpeed <= 0.f)
		{
			Storm::throwException<Storm::Exception>("Fluid " + std::to_string(fluidConfig._fluidId) + " sound of speed (" + std::to_string(fluidConfig._dynamicViscosity) + "m/s) is invalid!");
		}

		fluidConfig._cinematicViscosity = fluidConfig._dynamicViscosity / fluidConfig._density;
	}
	else if (generalConfigHolder.getConfig()._generalSimulationConfig._allowNoFluid)
	{
		sceneSimulationConfig._hasFluid = false;
	}
	else
	{
		Storm::throwException<Storm::Exception>("You should have at least one Fluid inside the scene, unless you've specified the tag 'General.allowNoFluid' to true!");
	}

	/* RigidBodies */
	AnimationsKeeper animationsKeeper;

	auto &rigidBodiesConfigArray = _sceneConfig->_rigidBodiesConfig;
	float rbConfigAngularVelocityDampingTmp = -1.f;

	Storm::XmlReader::readDataInList(srcTree, "RigidBodies", "RigidBody", rigidBodiesConfigArray,
		[&rbConfigAngularVelocityDampingTmp](const auto &rigidBodyConfigXml, Storm::SceneRigidBodyConfig &rbConfig)
	{
		return
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "id", rbConfig._rigidBodyID) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "meshFile", rbConfig._meshFilePath) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "isStatic", rbConfig._static) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "staticFrictionCoeff", rbConfig._staticFrictionCoefficient) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "dynamicFrictionCoeff", rbConfig._dynamicFrictionCoefficient) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "restitutionCoeff", rbConfig._restitutionCoefficient) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "angularDamping", rbConfigAngularVelocityDampingTmp) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "isStatic", rbConfig._static) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "fixTranslation", rbConfig._isTranslationFixed) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "wall", rbConfig._isWall) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "mass", rbConfig._mass) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "viscosity", rbConfig._viscosity) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "layerCount", rbConfig._layerCount) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "layeringGeneration", rbConfig._layerGenerationMode, parseLayeringGenerationTechnique) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "pInsideRemovalTechnique", rbConfig._insideRbFluidDetectionMethodEnum, parseInsideRemovalTech) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "collisionType", rbConfig._collisionShape, parseCollisionType) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "animation", rbConfig._animationXmlPath) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "animationName", rbConfig._animationName) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "translation", rbConfig._translation, parseVector3Element) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "rotation", rbConfig._rotation, parseVector3Element) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "scale", rbConfig._scale, parseVector3Element) ||
			Storm::XmlReader::handleXml(rigidBodyConfigXml, "color", rbConfig._color, parseColor4Element)
			;
	},
		[&animationsKeeper, &macroConfig, &rigidBodiesConfigArray, &fluidConfig, &rbConfigAngularVelocityDampingTmp](Storm::SceneRigidBodyConfig &rbConfig)
	{
		macroConfig(rbConfig._meshFilePath);
		macroConfig(rbConfig._animationXmlPath);
		macroConfig(rbConfig._animationName);

		if (rbConfig._rigidBodyID == std::numeric_limits<decltype(rbConfig._rigidBodyID)>::max())
		{
			Storm::throwException<Storm::Exception>("Rigid body id should be set using 'id' tag!");
		}

		// Minus 1 because of course, the rbConfig that we are currently filling has the same id than itself... 
		const auto lastToCheck = std::end(rigidBodiesConfigArray) - 1;
		if (const auto found = std::find_if(std::begin(rigidBodiesConfigArray), lastToCheck, [&rbConfig](const Storm::SceneRigidBodyConfig &registeredRb)
		{
			return registeredRb._rigidBodyID == rbConfig._rigidBodyID;
		}); found != lastToCheck)
		{
			Storm::throwException<Storm::Exception>("RigidBody id " + std::to_string(rbConfig._rigidBodyID) + " is already used!");
		}
		else if (rbConfig._rigidBodyID == fluidConfig._fluidId)
		{
			Storm::throwException<Storm::Exception>("RigidBody id " + std::to_string(rbConfig._rigidBodyID) + " is already being used by fluid data!");
		}
		else if (rbConfig._mass <= 0.f)
		{
			Storm::throwException<Storm::Exception>("mass " + std::to_string(rbConfig._mass) + "kg is invalid (rigid body " + std::to_string(rbConfig._rigidBodyID) + ")!");
		}
		else if (rbConfig._viscosity < 0.f)
		{
			Storm::throwException<Storm::Exception>("viscosity " + std::to_string(rbConfig._viscosity) + "Pa.s is invalid (rigid body " + std::to_string(rbConfig._rigidBodyID) + ")!");
		}
		else if (rbConfig._isWall && rbConfig._insideRbFluidDetectionMethodEnum != Storm::InsideParticleRemovalTechnique::None)
		{
			Storm::throwException<Storm::Exception>("Setting a insider particles removal technique for a wall rigidbody is forbidden (rigid body " + std::to_string(rbConfig._rigidBodyID) + ")!");
		}
		else if (rbConfig._layerCount == 0)
		{
			Storm::throwException<Storm::Exception>("The rigid body layer count is invalid (rigid body " + std::to_string(rbConfig._rigidBodyID) + ")! Value was " + std::to_string(rbConfig._layerCount));
		}
		else if (Storm::ColorCheckerHelper::isInvalid(rbConfig._color))
		{
			Storm::throwException<Storm::Exception>("The rigid body " + std::to_string(rbConfig._rigidBodyID) + " color is invalid! Value was " + Storm::toStdString(rbConfig._color));
		}
		
		if (rbConfig._isTranslationFixed && (rbConfig._isWall || rbConfig._static))
		{
			LOG_WARNING << "The rigid body " << rbConfig._rigidBodyID + " is not dynamic! Therefore the translation fixed flag set to true will be ignored.";
		}

		if (rbConfigAngularVelocityDampingTmp != -1.f)
		{
			if (rbConfig._isWall || rbConfig._static)
			{
				Storm::throwException<Storm::Exception>("RigidBody id " + std::to_string(rbConfig._rigidBodyID) + " is static. It cannot have a velocity damping coefficient!");
			}
			else if (rbConfigAngularVelocityDampingTmp > 1.f)
			{
				Storm::throwException<Storm::Exception>("The angular velocity damping value couldn't exceed 1.0 (rigid body " + std::to_string(rbConfig._rigidBodyID) + ")! Value was " + std::to_string(rbConfigAngularVelocityDampingTmp));
			}

			rbConfig._angularVelocityDamping = rbConfigAngularVelocityDampingTmp;
		}

		if (rbConfig._collisionShape == Storm::CollisionType::IndividualParticle)
		{
			if (!rbConfig._meshFilePath.empty())
			{
				Storm::throwException<Storm::Exception>("Specified a mesh (" + rbConfig._meshFilePath + ") while we spawn an individual rigid body particle! It is not allowed.");
			}

			LOG_DEBUG_WARNING <<
				"We'll spawn an individual rigid body particle (rigid body " + std::to_string(rbConfig._rigidBodyID) + ").\n"
				"Some config settings won't be applied.";
		}
		else
		{
			if (!std::filesystem::is_regular_file(rbConfig._meshFilePath))
			{
				Storm::throwException<Storm::Exception>("'" + rbConfig._meshFilePath + "' is not a valid mesh file!");
			}
		}

		// This is not the responsibility to the config module to parse and produce the animation.
		// This is the task of the animation module, therefore we'll forward to it the animation content without touching it.
		// Just having an animation content should be enough to flag to others module that an animation was made. Therefore this is the only thing we'll precompute here.
		if (!rbConfig._animationXmlPath.empty())
		{
			if (rbConfig._isWall || rbConfig._static)
			{
				Storm::throwException<Storm::Exception>("The rigid body " + std::to_string(rbConfig._rigidBodyID) + " is static or a wall, therefore cannot move along an animation!");
			}
			else if (!std::filesystem::is_regular_file(rbConfig._animationXmlPath))
			{
				Storm::throwException<Storm::Exception>("The rigid body " + std::to_string(rbConfig._rigidBodyID) + " animation xml path ('" + rbConfig._animationXmlPath + "') doesn't exist or is not a file!");
			}

			bool found = false;
			const boost::property_tree::ptree &animationXml = animationsKeeper.retrieveAnimationXml(rbConfig._animationXmlPath);
			for (const auto &rbAnimXml : animationXml)
			{
				if (rbAnimXml.first == "RigidBody")
				{
					bool animTagValid = false;

					std::string animName;
					if (Storm::XmlReader::readXmlAttribute(rbAnimXml.second, animName, "name"))
					{
						animTagValid = true;
						if (!animName.empty() && rbConfig._animationName == animName)
						{
							rbConfig._animationXmlContent = Storm::toStdString<XmlAnimationParserPolicy>(rbAnimXml.second);
							found = true;
							break;
						}
					}

					decltype(rbConfig._rigidBodyID) rbAnimId;
					if (Storm::XmlReader::readXmlAttribute(rbAnimXml.second, rbAnimId, "id"))
					{
						animTagValid = true;
						if (rbAnimId == rbConfig._rigidBodyID)
						{
							if (found)
							{
								Storm::throwException<Storm::Exception>("The rigid body " + std::to_string(rbConfig._rigidBodyID) + " animation was found in two different place inside the xml file " + rbConfig._animationXmlPath + "! Please, merge both animations!");
							}

							rbConfig._animationXmlContent = Storm::toStdString<XmlAnimationParserPolicy>(rbAnimXml.second);
							found = true;
						}
					}

					if (!animTagValid)
					{
						Storm::throwException<Storm::Exception>("One of the animations found inside the xml file " + rbConfig._animationXmlPath + " doesn't have an id or a name! It must have one or both!");
					}
				}
				else
				{
					LOG_ERROR << "tag '" << rbAnimXml.first << "' inside " + rbConfig._animationXmlPath + " is unknown, therefore it cannot be handled";
				}
			}

			if (!found)
			{
				Storm::throwException<Storm::Exception>("The animation xml file '" + rbConfig._animationXmlPath + "' doesn't contain an animation for rigid body " + std::to_string(rbConfig._rigidBodyID) + "!");
			}
		}
	});

	/* Contraints */
	unsigned int contraintsIndex = 0;
	Storm::XmlReader::readDataInList(srcTree, "Constraints", "Constraint", _sceneConfig->_contraintsConfig,
		[](const auto &constraintConfigXml, Storm::SceneConstraintConfig &contraintConfig)
	{
		return
			Storm::XmlReader::handleXml(constraintConfigXml, "rbId1", contraintConfig._rigidBodyId1) ||
			Storm::XmlReader::handleXml(constraintConfigXml, "rbId2", contraintConfig._rigidBodyId2) ||
			Storm::XmlReader::handleXml(constraintConfigXml, "length", contraintConfig._constraintsLength) ||
			Storm::XmlReader::handleXml(constraintConfigXml, "visualize", contraintConfig._shouldVisualize) ||
			Storm::XmlReader::handleXml(constraintConfigXml, "noRotation", contraintConfig._preventRotations) ||
			Storm::XmlReader::handleXml(constraintConfigXml, "rb1LinkOffset", contraintConfig._rigidBody1LinkTranslationOffset, parseVector3Element) ||
			Storm::XmlReader::handleXml(constraintConfigXml, "rb2LinkOffset", contraintConfig._rigidBody2LinkTranslationOffset, parseVector3Element)
			;
	},
		[&rigidBodiesConfigArray, &contraintsIndex](Storm::SceneConstraintConfig &contraintConfig)
	{
		contraintConfig._constraintId = contraintsIndex++;
		const auto lastToCheck = std::end(rigidBodiesConfigArray);

		if (contraintConfig._constraintsLength == -1.f)
		{
			Storm::throwException<Storm::Exception>("Constraints " + std::to_string(contraintsIndex) + " length isn't set but is a mandatory setting, please specify it with 'length' tag.");
		}
		else if (contraintConfig._constraintsLength <= 0.f)
		{
			Storm::throwException<Storm::Exception>("Constraints " + std::to_string(contraintsIndex) + " length should be a positive non zero value (was " + std::to_string(contraintConfig._constraintsLength) + ")!");
		}
		else if (contraintConfig._rigidBodyId1 == std::numeric_limits<decltype(contraintConfig._rigidBodyId1)>::max())
		{
			Storm::throwException<Storm::Exception>("Constraints " + std::to_string(contraintsIndex) + " rigid body 1 id wasn't set! It is a mandatory field.");
		}
		else if (contraintConfig._rigidBodyId2 == std::numeric_limits<decltype(contraintConfig._rigidBodyId2)>::max())
		{
			Storm::throwException<Storm::Exception>("Constraints " + std::to_string(contraintsIndex) + " rigid body 2 id wasn't set! It is a mandatory field.");
		}
		else if (std::find_if(std::begin(rigidBodiesConfigArray), lastToCheck, [&contraintConfig](const Storm::SceneRigidBodyConfig &registeredRb)
		{
			return registeredRb._rigidBodyID == contraintConfig._rigidBodyId1;
		}) == lastToCheck)
		{
			Storm::throwException<Storm::Exception>("Constraints " + std::to_string(contraintsIndex) + " rigid body 1 id wasn't found (" + std::to_string(contraintConfig._rigidBodyId1) + ")!");
		}
		else if (std::find_if(std::begin(rigidBodiesConfigArray), lastToCheck, [&contraintConfig](const Storm::SceneRigidBodyConfig &registeredRb)
		{
			return registeredRb._rigidBodyID == contraintConfig._rigidBodyId2;
		}) == lastToCheck)
		{
			Storm::throwException<Storm::Exception>("Constraints " + std::to_string(contraintsIndex) + " rigid body 2 id wasn't found (" + std::to_string(contraintConfig._rigidBodyId2) + ")!");
		}
	});

	/* Blowers */
	auto &blowersConfigArray = _sceneConfig->_blowersConfig;

	Storm::XmlReader::readDataInList(srcTree, "Blowers", "Blower", blowersConfigArray,
		[](const auto &blowerConfigXml, Storm::SceneBlowerConfig &blowerConfig)
	{
		return
			Storm::XmlReader::handleXml(blowerConfigXml, "id", blowerConfig._blowerId) ||
			Storm::XmlReader::handleXml(blowerConfigXml, "type", blowerConfig._blowerType, parseBlowerType) ||
			Storm::XmlReader::handleXml(blowerConfigXml, "startTime", blowerConfig._startTimeInSeconds) ||
			Storm::XmlReader::handleXml(blowerConfigXml, "endTime", blowerConfig._stopTimeInSeconds) ||
			Storm::XmlReader::handleXml(blowerConfigXml, "fadeInTime", blowerConfig._fadeInTimeInSeconds) ||
			Storm::XmlReader::handleXml(blowerConfigXml, "fadeOutTime", blowerConfig._fadeOutTimeInSeconds) ||
			Storm::XmlReader::handleXml(blowerConfigXml, "radius", blowerConfig._radius) ||
			Storm::XmlReader::handleXml(blowerConfigXml, "upRadius", blowerConfig._upRadius) ||
			Storm::XmlReader::handleXml(blowerConfigXml, "downRadius", blowerConfig._downRadius) ||
			Storm::XmlReader::handleXml(blowerConfigXml, "height", blowerConfig._height) ||
			Storm::XmlReader::handleXml(blowerConfigXml, "dimension", blowerConfig._blowerDimension, parseVector3Element) ||
			Storm::XmlReader::handleXml(blowerConfigXml, "force", blowerConfig._blowerForce, parseVector3Element) ||
			Storm::XmlReader::handleXml(blowerConfigXml, "position", blowerConfig._blowerPosition, parseVector3Element)
			;
	},
		[&rigidBodiesConfigArray, &fluidConfig](Storm::SceneBlowerConfig &blowerConfig)
	{
		if (blowerConfig._blowerId == std::numeric_limits<decltype(blowerConfig._blowerId)>::max())
		{
			Storm::throwException<Storm::Exception>("Blower id should be set using 'id' tag!");
		}
		else if (blowerConfig._blowerId == fluidConfig._fluidId)
		{
			Storm::throwException<Storm::Exception>("Blower with id " + std::to_string(blowerConfig._blowerId) + " shares the same id than an already registered fluid. It is forbidden!");
		}
		else if (blowerConfig._startTimeInSeconds < 0.f)
		{
			Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " start time is invalid (it cannot be lesser or equal to 0, value was " + std::to_string(blowerConfig._startTimeInSeconds) + ")!");
		}
		else if (blowerConfig._stopTimeInSeconds == -1.f && blowerConfig._fadeOutTimeInSeconds > 0.f)
		{
			Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " doesn't have a stop time but has a fade out time. It is illogical!");
		}
		else if (blowerConfig._fadeInTimeInSeconds < 0.f)
		{
			Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " fade in time cannot be negative!");
		}
		else if (blowerConfig._fadeOutTimeInSeconds < 0.f)
		{
			Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " fade out time cannot be negative!");
		}
		else if (blowerConfig._stopTimeInSeconds != -1.f)
		{
			if (blowerConfig._startTimeInSeconds >= blowerConfig._stopTimeInSeconds)
			{
				Storm::throwException<Storm::Exception>(
					"Blower " + std::to_string(blowerConfig._blowerId) + " end time cannot be before its start time.\n"
					"Either set it to -1 to specify that there is no stop time, or set it strictly greater than the start time!\n"
					"startTime was " + std::to_string(blowerConfig._startTimeInSeconds) + "s\n"
					"endTime was " + std::to_string(blowerConfig._stopTimeInSeconds) + "s.");
			}
			else if (blowerConfig._stopTimeInSeconds < blowerConfig._fadeOutTimeInSeconds)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " fade out time is greater than the stop time (this means that it has faded out even before the time 0, which does not make much sense)!");
			}
			else if ((blowerConfig._startTimeInSeconds + blowerConfig._fadeInTimeInSeconds) > (blowerConfig._stopTimeInSeconds - blowerConfig._fadeOutTimeInSeconds))
			{
				Storm::throwException<Storm::Exception>(
					"Blower " + std::to_string(blowerConfig._blowerId) + " fade in and fade out overlaps... Too complex and error prone, please, change it to a way those do not overlaps!\n"
					"Fade in time start=" + std::to_string(blowerConfig._startTimeInSeconds) + "s; end=" + std::to_string(blowerConfig._startTimeInSeconds + blowerConfig._fadeInTimeInSeconds) + "s.\n"
					"Fade out time start=" + std::to_string(blowerConfig._stopTimeInSeconds - blowerConfig._fadeOutTimeInSeconds) + "s; end=" + std::to_string(blowerConfig._stopTimeInSeconds) + "s.");
			}
		}

		const auto lastRbToCheck = std::end(rigidBodiesConfigArray);
		if (const auto found = std::find_if(std::begin(rigidBodiesConfigArray), lastRbToCheck, [id = blowerConfig._blowerId](const Storm::SceneRigidBodyConfig &registeredRb)
		{
			return registeredRb._rigidBodyID == id;
		}); found != lastRbToCheck)
		{
			Storm::throwException<Storm::Exception>("Blower with id " + std::to_string(blowerConfig._blowerId) + " shares the same id than an already registered rigid body. It is forbidden!");
		}

		switch (blowerConfig._blowerType)
		{
		case Storm::BlowerType::Cube:
		case Storm::BlowerType::CubeGradualDirectional:
			if (blowerConfig._blowerDimension == Storm::Vector3::Zero())
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a cube) should have defined a dimension!");
			}
			else if (blowerConfig._blowerDimension.x() <= 0.f || blowerConfig._blowerDimension.y() <= 0.f || blowerConfig._blowerDimension.z() <= 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a cube) cannot have one of its dimension value lesser or equal to 0! Specified dimension was " + Storm::toStdString(blowerConfig._blowerDimension));
			}
			else if (blowerConfig._radius != 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a cube) cannot have a radius (" + Storm::toStdString(blowerConfig._radius) + ")!");
			}
			else if (blowerConfig._height != 0.f && blowerConfig._height != blowerConfig._blowerDimension.y())
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a cube) cannot have a specific height (use the dimension for a cube, not the height tag (" + Storm::toStdString(blowerConfig._height) + "))!");
			}
			else if (blowerConfig._upRadius != -1.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a cube) cannot have an up radius (" + Storm::toStdString(blowerConfig._upRadius) + ")!");
			}
			else if (blowerConfig._downRadius != -1.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a cube) cannot have a down radius (" + Storm::toStdString(blowerConfig._downRadius) + ")!");
			}
			break;

		case Storm::BlowerType::Sphere:
		case Storm::BlowerType::SpherePlanarGradual:
		case Storm::BlowerType::RepulsionSphere:
		case Storm::BlowerType::ExplosionSphere:
			if (blowerConfig._radius <= 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a sphere) should have defined a positive non-zero radius!");
			}
			else if (blowerConfig._height != 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a sphere) cannot have a specific height (this tag is reserved for cylinder derived blowers (" + Storm::toStdString(blowerConfig._height) + "))!");
			}
			else if (blowerConfig._upRadius != -1.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a sphere) cannot have an up radius (" + Storm::toStdString(blowerConfig._upRadius) + ")!");
			}
			else if (blowerConfig._downRadius != -1.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a sphere) cannot have a down radius (" + Storm::toStdString(blowerConfig._downRadius) + ")!");
			}
			blowerConfig._blowerDimension = Storm::Vector3{ blowerConfig._radius, blowerConfig._radius, blowerConfig._radius };
			break;

		case Storm::BlowerType::PulseExplosionSphere:
			if (blowerConfig._radius <= 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a pulse explosion) should have defined a positive non-zero radius!");
			}
			else if (blowerConfig._height != 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a pulse explosion) cannot have a specific height (this tag is reserved for cylinder derived blowers (" + Storm::toStdString(blowerConfig._height) + "))!");
			}
			else if (blowerConfig._stopTimeInSeconds != -1.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a pulse explosion) have its stop time set in stone! You cannot override it (you've set " + std::to_string(blowerConfig._stopTimeInSeconds) + "s).");
			}
			else if (blowerConfig._fadeInTimeInSeconds != 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a pulse explosion) have its fadeInTime set in stone! You cannot override it (you've set " + std::to_string(blowerConfig._fadeInTimeInSeconds) + "s).");
			}
			else if (blowerConfig._fadeOutTimeInSeconds != 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a pulse explosion) have its fadeInTime set in stone! You cannot override it (you've set " + std::to_string(blowerConfig._fadeOutTimeInSeconds) + "s).");
			}
			else if (blowerConfig._upRadius != -1.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a pulse explosion) cannot have an up radius (" + Storm::toStdString(blowerConfig._upRadius) + ")!");
			}
			else if (blowerConfig._downRadius != -1.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a pulse explosion) cannot have a down radius (" + Storm::toStdString(blowerConfig._downRadius) + ")!");
			}

			blowerConfig._blowerDimension = Storm::Vector3{ blowerConfig._radius, blowerConfig._radius, blowerConfig._radius };
			break;

		case Storm::BlowerType::Cylinder:
		case Storm::BlowerType::CylinderGradualMidPlanar:
			if (blowerConfig._radius <= 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a cylinder) should have defined a positive non-zero radius!");
			}
			else if (blowerConfig._height <= 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a cylinder) should have defined a positive non zero height!");
			}
			else if (blowerConfig._blowerDimension.x() != 0.f || blowerConfig._blowerDimension.z() != 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a cylinder) shouldn't use dimension tag to specify x and z width and depth but radius instead!");
			}
			else if (blowerConfig._blowerDimension.y() != 0.f && blowerConfig._blowerDimension.y() != blowerConfig._height)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a cylinder) shouldn't use dimension tag for the height but use height tag instead!");
			}
			else if (blowerConfig._upRadius != -1.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a cylinder) cannot have an up radius (" + Storm::toStdString(blowerConfig._upRadius) + ")!");
			}
			else if (blowerConfig._downRadius != -1.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a cylinder) cannot have a down radius (" + Storm::toStdString(blowerConfig._downRadius) + ")!");
			}

			blowerConfig._blowerDimension = Storm::Vector3{ 0.f, blowerConfig._height, 0.f };
			break;

		case Storm::BlowerType::Cone:
			if (blowerConfig._radius != 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a Cone) cannot have a radius (" + Storm::toStdString(blowerConfig._radius) + ")!");
			}
			else if (blowerConfig._height <= 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a Cone) should have defined a positive non zero height!");
			}
			else if (blowerConfig._blowerDimension.x() != 0.f || blowerConfig._blowerDimension.z() != 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a Cone) shouldn't use dimension tag to specify x and z width and depth but radius instead!");
			}
			else if (blowerConfig._blowerDimension.y() != 0.f && blowerConfig._blowerDimension.y() != blowerConfig._height)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a Cone) shouldn't use dimension tag for the height but use height tag instead!");
			}
			else if (blowerConfig._upRadius == -1.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a Cone) should have defined an up radius!");
			}
			else if (blowerConfig._downRadius == -1.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " (a Cone) should have defined a down radius!");
			}
			else if (blowerConfig._upRadius < 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + "(a Cone) up radius is invalid. It should be a non negative value (" + Storm::toStdString(blowerConfig._upRadius) + ")!");
			}
			else if (blowerConfig._downRadius < 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + "(a Cone) down radius is invalid. It should be a non negative value (" + Storm::toStdString(blowerConfig._downRadius) + ")!");
			}
			else if (blowerConfig._downRadius == 0.f && blowerConfig._upRadius == 0.f)
			{
				Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + "(a Cone) down and up radius are both zero. It is forbidden!");
			}

			blowerConfig._blowerDimension = Storm::Vector3{ 0.f, blowerConfig._height, 0.f };
			
			if (blowerConfig._downRadius == blowerConfig._upRadius)
			{
				LOG_WARNING <<
					"Blower " << std::to_string(blowerConfig._blowerId) << " have both the up and down radius equal (" << blowerConfig._downRadius << ").\n"
					"It is allowed but note that it will be in fact a cylinder. Except that Cylinders are more optimized to compute a cylinder effect area.\n";
			}
			break;

		case Storm::BlowerType::None:
			Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " should have defined a blower type, this is mandatory!");

		default:
			Storm::throwException<Storm::Exception>("Blower " + std::to_string(blowerConfig._blowerId) + " check wasn't implemented!");
		}
	});
}

const Storm::SceneConfig& Storm::SceneConfigHolder::getConfig() const
{
	return *_sceneConfig;
}

Storm::SceneConfig& Storm::SceneConfigHolder::getConfig()
{
	return *_sceneConfig;
}
