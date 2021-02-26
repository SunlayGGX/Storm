#include "AssetLoaderManager.h"

#include "AssimpLoggingWrapper.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "IGraphicsManager.h"
#include "IPhysicsManager.h"
#include "ISimulatorManager.h"
#include "IThreadManager.h"
#include "ISerializerManager.h"
#include "ITimeManager.h"
#include "IAnimationManager.h"

#include "SceneSimulationConfig.h"
#include "SceneFluidConfig.h"
#include "SceneBlowerConfig.h"
#include "SceneRigidBodyConfig.h"

#include "RigidBody.h"

#include "BasicMeshGenerator.h"

#include "FluidParticleLoadDenseMode.h"

#include "BlowerMeshMaker.h"
#include "BlowerType.h"

#include "ThreadEnumeration.h"
#include "ThreadFlagEnum.h"

#include "AssetCacheData.h"
#include "AssetCacheDataOrder.h"

#include "SceneRecordConfig.h"
#include "RecordMode.h"

#include "SerializeParticleSystemLayout.h"
#include "SerializeConstraintLayout.h"
#include "SerializeRecordHeader.h"

#include "StateLoadingOrders.h"
#include "SystemSimulationStateObject.h"
#include "SimulationState.h"

#include "CollisionType.h"

#include <Assimp\DefaultLogger.hpp>

#include <future>


namespace
{
	void initializeAssimpLogger()
	{
		Assimp::Logger* defaultLogger = Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
		if (!defaultLogger->attachStream(new Storm::AssimpDebugLoggerStream, Assimp::Logger::ErrorSeverity::Debugging))
		{
			LOG_WARNING << "Assimp Logger Debug stream failed to attach";
		}

		if (!defaultLogger->attachStream(new Storm::AssimpInfoLoggerStream, Assimp::Logger::ErrorSeverity::Info))
		{
			LOG_WARNING << "Assimp Logger Info stream failed to attach";
		}

		if (!defaultLogger->attachStream(new Storm::AssimpWarningLoggerStream, Assimp::Logger::ErrorSeverity::Warn))
		{
			LOG_WARNING << "Assimp Logger Warning stream failed to attach";
		}

		if (!defaultLogger->attachStream(new Storm::AssimpErrorLoggerStream, Assimp::Logger::ErrorSeverity::Err))
		{
			LOG_WARNING << "Assimp Logger Error stream failed to attach";
		}
	}

	template<Storm::FluidParticleLoadDenseMode>
	void computeParticleCountBoxExtents(const Storm::SceneFluidBlockConfig &fluidBlock, const float particleDiameter, std::size_t &outParticleXCount, std::size_t &outParticleYCount, std::size_t &outParticleZCount);

	template<Storm::FluidParticleLoadDenseMode>
	void generateFluidParticles(std::vector<Storm::Vector3> &inOutFluidParticlePos, const Storm::SceneFluidBlockConfig &fluidBlockGenerated, const float particleDiameter);


	template<>
	void computeParticleCountBoxExtents<Storm::FluidParticleLoadDenseMode::Normal>(const Storm::SceneFluidBlockConfig &fluidBlock, const float particleDiameter, std::size_t &outParticleXCount, std::size_t &outParticleYCount, std::size_t &outParticleZCount)
	{
		outParticleXCount = static_cast<std::size_t>(fabs(fluidBlock._firstPoint.x() - fluidBlock._secondPoint.x()) / particleDiameter);
		outParticleYCount = static_cast<std::size_t>(fabs(fluidBlock._firstPoint.y() - fluidBlock._secondPoint.y()) / particleDiameter);
		outParticleZCount = static_cast<std::size_t>(fabs(fluidBlock._firstPoint.z() - fluidBlock._secondPoint.z()) / particleDiameter);
	}

	template<>
	void computeParticleCountBoxExtents<Storm::FluidParticleLoadDenseMode::AsSplishSplash>(const Storm::SceneFluidBlockConfig &fluidBlock, const float particleDiameter, std::size_t &outParticleXCount, std::size_t &outParticleYCount, std::size_t &outParticleZCount)
	{
		const Storm::Vector3 diff = fluidBlock._secondPoint - fluidBlock._firstPoint;

		outParticleXCount = static_cast<std::size_t>(std::roundf(diff[0] / particleDiameter)) - 1ULL;
		outParticleYCount = static_cast<std::size_t>(std::roundf(diff[1] / particleDiameter)) - 1ULL;
		outParticleZCount = static_cast<std::size_t>(std::roundf(diff[2] / particleDiameter)) - 1ULL;
	}

	template<>
	void generateFluidParticles<Storm::FluidParticleLoadDenseMode::Normal>(std::vector<Storm::Vector3> &inOutFluidParticlePos, const Storm::SceneFluidBlockConfig &fluidBlockGenerated, const float particleDiameter)
	{
		std::size_t particleXCount;
		std::size_t particleYCount;
		std::size_t particleZCount;

		computeParticleCountBoxExtents<Storm::FluidParticleLoadDenseMode::Normal>(fluidBlockGenerated, particleDiameter, particleXCount, particleYCount, particleZCount);

		const float xPosBegin = std::min(fluidBlockGenerated._firstPoint.x(), fluidBlockGenerated._secondPoint.x());
		const float yPosBegin = std::min(fluidBlockGenerated._firstPoint.y(), fluidBlockGenerated._secondPoint.y());
		const float zPosBegin = std::min(fluidBlockGenerated._firstPoint.z(), fluidBlockGenerated._secondPoint.z());

		Storm::Vector3 currentParticlePos;
		for (std::size_t xIter = 0; xIter < particleXCount; ++xIter)
		{
			currentParticlePos.x() = xPosBegin + static_cast<float>(xIter) * particleDiameter;
			for (std::size_t yIter = 0; yIter < particleYCount; ++yIter)
			{
				currentParticlePos.y() = yPosBegin + static_cast<float>(yIter) * particleDiameter;
				for (std::size_t zIter = 0; zIter < particleZCount; ++zIter)
				{
					currentParticlePos.z() = zPosBegin + static_cast<float>(zIter) * particleDiameter;
					inOutFluidParticlePos.push_back(currentParticlePos);
				}
			}
		}
	}

	template<>
	void generateFluidParticles<Storm::FluidParticleLoadDenseMode::AsSplishSplash>(std::vector<Storm::Vector3> &inOutFluidParticlePos, const Storm::SceneFluidBlockConfig &fluidBlockGenerated, const float particleDiameter)
	{
		std::size_t particleXCount;
		std::size_t particleYCount;
		std::size_t particleZCount;

		computeParticleCountBoxExtents<Storm::FluidParticleLoadDenseMode::AsSplishSplash>(fluidBlockGenerated, particleDiameter, particleXCount, particleYCount, particleZCount);

		const Storm::Vector3 start = fluidBlockGenerated._firstPoint + particleDiameter * Storm::Vector3::Ones();

		for (std::size_t xIter = 0; xIter < particleXCount; ++xIter)
		{
			const float xCoeff = static_cast<float>(xIter);
			for (std::size_t yIter = 0; yIter < particleYCount; ++yIter)
			{
				const float yCoeff = static_cast<float>(yIter);
				for (std::size_t zIter = 0; zIter < particleZCount; ++zIter)
				{
					const float zCoeff = static_cast<float>(zIter);
					Storm::Vector3 &addedPos = inOutFluidParticlePos.emplace_back(xCoeff * particleDiameter, yCoeff * particleDiameter, zCoeff * particleDiameter);
					addedPos += start;
				}
			}
		}
	}

	template<class FutureContainerType>
	void waitForFutures(FutureContainerType &inOutFutures)
	{
		std::string errorMsg;

		const std::size_t futureCount = inOutFutures.size();
		for (std::size_t iter = 0; iter < futureCount; ++iter)
		{
			auto &currentFutureToWait = inOutFutures[iter];
			try
			{
				currentFutureToWait.get();
			}
			catch (const Storm::Exception &e)
			{
				const std::string iterStr = std::to_string(iter);
				const std::string_view exceptionMsg = e.what();
				const std::string_view exceptionStackTrace = e.stackTrace();

				errorMsg.reserve(errorMsg.size() + iterStr.size() + exceptionMsg.size() + exceptionStackTrace.size() + 56);

				errorMsg += "Future ";
				errorMsg += iterStr;
				errorMsg += " has thrown an exception : ";
				errorMsg += exceptionMsg;
				errorMsg += ".\nStackTrace:\n";
				errorMsg += exceptionStackTrace;
				errorMsg += '\n';
			}
			catch (const std::exception &e)
			{
				const std::string iterStr = std::to_string(iter);
				const std::string_view exceptionMsg = e.what();

				errorMsg.reserve(errorMsg.size() + iterStr.size() + exceptionMsg.size() + 56);

				errorMsg += "Future ";
				errorMsg += iterStr;
				errorMsg += " has thrown an exception : ";
				errorMsg += exceptionMsg;
				errorMsg += ".\n";
			}
			catch (...)
			{
				const std::string iterStr = std::to_string(iter);

				errorMsg.reserve(errorMsg.size() + iterStr.size() + 56);

				errorMsg += "Future ";
				errorMsg += iterStr;
				errorMsg += " has thrown an unknown exception!\n";
			}
		}

		inOutFutures.clear();

		if (!errorMsg.empty())
		{
			Storm::throwException<Storm::Exception>(errorMsg);
		}
	}

	void loadState(Storm::StateLoadingOrders &inOutLoadingStateOrder)
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

		Storm::StateLoadingOrders::LoadingSettings &settings = inOutLoadingStateOrder._settings;

		bool loadPhysicsTime;
		bool loadForces;
		bool loadVelocities;
		configMgr.stateShouldLoad(loadPhysicsTime, loadForces, loadVelocities);

		settings._loadPhysicsTime = loadPhysicsTime;
		settings._loadForces = loadForces;
		settings._loadVelocities = loadVelocities;

		Storm::ISerializerManager &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();
		serializerMgr.loadState(inOutLoadingStateOrder);

		// Provide a non existent protection, but at least, log it.
		// Throwing is not the solution, we hope user know what it does but if it doesn't, then we warn him.
		// If we throw, user that really wants it will just rename the config file. This is too easily bypassed
		// (and no warning will be issued then because we don't look at the content of the config file (it is the purpose of this feature to reload from a state and
		// try with different settings)), therefore, preventing the user with a throw is just more a pain in the ass than what is truly gained. 
		// And I'm not a pro to needlessly overcomplexify things that could just be much more simpler for everyone... ;)
		const std::string &currentSceneName = configMgr.getSceneName();
		const std::string &stateSceneName = inOutLoadingStateOrder._simulationState->_configSceneName;
		if (stateSceneName != currentSceneName)
		{
			LOG_WARNING <<
				"We loaded a state made for another scene (" << stateSceneName << ") but loaded it into scene " << currentSceneName << ".\n"
				"Be aware that this could produce unexpected results, behaviors and bugs."
				;
		}
	}
}

#define STORM_EXECUTE_CASE_ON_DENSE_MODE(DenseModeEnum) case Storm::##DenseModeEnum: STORM_EXECUTE_METHOD_ON_DENSE_MODE(Storm::##DenseModeEnum) break


Storm::AssetLoaderManager::AssetLoaderManager() = default;
Storm::AssetLoaderManager::~AssetLoaderManager() = default;

void Storm::AssetLoaderManager::initialize_Implementation()
{
	LOG_COMMENT << "Asset loading started! Depending on the caching state of each assets, it could take some time...";

	initializeAssimpLogger();

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	switch (configMgr.getSceneRecordConfig()._recordMode)
	{
	case Storm::RecordMode::Replay:
		this->initializeForReplay();
		break;

	case Storm::RecordMode::Record:
	case Storm::RecordMode::None:
		this->initializeForSimulation();
		break;

	default:
		Storm::throwException<Storm::Exception>("Unhandled initialization mode from specified record mode!");
	}

	LOG_DEBUG << "Cleaning asset loading cache data";
	this->clearCachedAssetData();
	_assetSpecificMutexMap.clear();

	LOG_COMMENT << "Asset loading finished!";
}

void Storm::AssetLoaderManager::cleanUp_Implementation()
{
	Assimp::DefaultLogger::kill();
}

void Storm::AssetLoaderManager::initializeForReplay()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::ISerializerManager &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();
	Storm::ISimulatorManager &simulMgr = singletonHolder.getSingleton<Storm::ISimulatorManager>();
	Storm::IGraphicsManager &graphicsMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();
	//Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	const Storm::SerializeRecordHeader &recordHeader = serializerMgr.beginReplay();

	LOG_COMMENT << "Record header read. There is " << recordHeader._frameCount << " frames recorded at " << recordHeader._recordFrameRate << " fps.";

	for (const Storm::SerializeParticleSystemLayout &systemLayout : recordHeader._particleSystemLayouts)
	{
		if (systemLayout._isFluid)
		{
			simulMgr.addFluidParticleSystem(systemLayout._particleSystemId, systemLayout._particlesCount);
		}
		else
		{
			// It is also a way to find out that the recording to be replayed match a minima the scene we use to load it (we should have at least the right rigid bodies).
			const Storm::SceneRigidBodyConfig &associatedRbConfig = configMgr.getSceneRigidBodyConfig(systemLayout._particleSystemId);
			auto &emplacedRb = _rigidBodies.emplace_back(std::static_pointer_cast<Storm::IRigidBody>(std::make_shared<Storm::RigidBody>(associatedRbConfig, Storm::RigidBody::ReplayMode{})));

			graphicsMgr.bindParentRbToMesh(systemLayout._particleSystemId, emplacedRb);
			//physicsMgr.bindParentRbToPhysicalBody(associatedRbConfig, emplacedRb);

			simulMgr.addRigidBodyParticleSystem(systemLayout._particleSystemId, systemLayout._particlesCount);
		}
	}

	/* Loading Blowers */
	const auto &blowersConfigToLoad = configMgr.getSceneBlowersConfig();

	std::vector<Storm::Vector3> areaVertexesTmp;
	std::vector<uint32_t> areaIndexesTmp;
	for (const Storm::SceneBlowerConfig &blowerToLoad : blowersConfigToLoad)
	{
		// Generate the mesh area data to pass to the graphical resources.
#define STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(BlowerTypeName, BlowerTypeXmlName, EffectAreaType, MeshMakerType) \
case Storm::BlowerType::BlowerTypeName: \
	Storm::MeshMakerType::generate(blowerToLoad, areaVertexesTmp, areaIndexesTmp); \
	break;

		switch (blowerToLoad._blowerType)
		{
			STORM_XMACRO_GENERATE_BLOWERS_CODE;

		default:
		case Storm::BlowerType::None:
			Storm::throwException<Storm::Exception>("Unknown Blower to be created!");
			break;
		}

#undef STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER

		threadMgr.executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [&graphicsMgr, &blowerToLoad, areaVertexes = std::move(areaVertexesTmp), areaIndexes = std::move(areaIndexesTmp)]()
		{
			graphicsMgr.loadBlower(blowerToLoad, areaVertexes, areaIndexes);
		});

		simulMgr.loadBlower(blowerToLoad);
	}

	Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
	for (const Storm::SerializeConstraintLayout &constraintLayout : recordHeader._contraintLayouts)
	{
		physicsMgr.loadRecordedConstraint(constraintLayout);
	}
}

void Storm::AssetLoaderManager::initializeForSimulation()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	std::filesystem::create_directories(Storm::RigidBody::retrieveParticleDataCacheFolder());

	Storm::IGraphicsManager &graphicsMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();
	Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();
	Storm::ISimulatorManager &simulMgr = singletonHolder.getSingleton<Storm::ISimulatorManager>();
	Storm::IThreadManager &threadMgr = singletonHolder.getSingleton<Storm::IThreadManager>();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	Storm::StateLoadingOrders loadedState;

	const std::string &stateFileToLoad = configMgr.getStateFilePath();
	const bool shouldLoadState = !stateFileToLoad.empty();
	if (shouldLoadState)
	{
		LOG_DEBUG << "State file specified, we'll load from it.";

		loadedState._settings._filePath = stateFileToLoad;
		loadState(loadedState);
	}

	/* Load rigid bodies */
	LOG_COMMENT << "Loading Rigid body";

	std::vector<std::future<void>> asyncLoadingArray;

	const auto &rigidBodiesConfigToLoad = configMgr.getSceneRigidBodiesConfig();
	const std::size_t rigidBodyCount = rigidBodiesConfigToLoad.size();

	if (rigidBodyCount > 0)
	{
		_rigidBodies.reserve(rigidBodyCount);
		asyncLoadingArray.reserve(rigidBodyCount);

		Storm::SimulationState &state = *loadedState._simulationState;

		if (shouldLoadState)
		{
			const auto endRigidBodiesDataToLoadConfigIter = std::end(rigidBodiesConfigToLoad);
			for (const Storm::SystemSimulationStateObject &pSystem : state._pSystemStates)
			{
				if (!pSystem._isFluid)
				{
					if (auto found = std::find_if(std::begin(rigidBodiesConfigToLoad), endRigidBodiesDataToLoadConfigIter, [id = pSystem._id](const Storm::SceneRigidBodyConfig &rbConfig)
					{
						return rbConfig._rigidBodyID == id;
					}); found == endRigidBodiesDataToLoadConfigIter)
					{
						Storm::throwException<Storm::Exception>("Cannot link loaded particle system to one we set from the configuration !");
					}
				}
			}
		}

		for (const auto &rbToLoad : rigidBodiesConfigToLoad)
		{
			asyncLoadingArray.emplace_back(std::async(std::launch::async, [this, &graphicsMgr, &physicsMgr, &rbToLoad, &state, shouldLoadState]()
			{
				Storm::SystemSimulationStateObject* rbStateFound = nullptr;
				if (shouldLoadState)
				{
					const auto endStatesIter = std::end(state._pSystemStates);
					if (auto found = std::find_if(std::begin(state._pSystemStates), endStatesIter, [id = rbToLoad._rigidBodyID](const auto &rbPSystemState)
					{
						return rbPSystemState._id == id;
					}); found != endStatesIter)
					{
						rbStateFound = &*found;
						if (rbStateFound->_isFluid)
						{
							Storm::throwException<Storm::Exception>("The state " + std::to_string(rbToLoad._rigidBodyID) + " we're trying to load as a rigid body is in fact the state of a fluid system! It isn't allowed!");
						}
					}
				}
				
				std::shared_ptr<Storm::IRigidBody> loadedRigidBody;
				if (rbStateFound != nullptr)
				{
					Storm::SceneRigidBodyConfig overridenRbConfig = rbToLoad;
					overridenRbConfig._translation = rbStateFound->_globalPosition;

					loadedRigidBody = std::static_pointer_cast<Storm::IRigidBody>(std::make_shared<Storm::RigidBody>(overridenRbConfig, std::move(*rbStateFound)));
				}
				else
				{
					loadedRigidBody = std::static_pointer_cast<Storm::IRigidBody>(std::make_shared<Storm::RigidBody>(rbToLoad));
				}

				const unsigned int emplacedRbId = loadedRigidBody->getRigidBodyID();

				{
					std::lock_guard<std::mutex> lock{ _addingMutex };
					auto &emplacedRb = _rigidBodies.emplace_back(std::move(loadedRigidBody));

					graphicsMgr.bindParentRbToMesh(emplacedRbId, emplacedRb);
					physicsMgr.bindParentRbToPhysicalBody(rbToLoad, emplacedRb);
				}

				LOG_DEBUG << "Rigid body " << emplacedRbId << " created and bound to the right modules.";
			}));
		}

		/* Loading animations */
		Storm::IAnimationManager &animMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IAnimationManager>();
		for (const auto &rbToLoad : rigidBodiesConfigToLoad)
		{
			if (!rbToLoad._animationXmlPath.empty())
			{
				animMgr.createAnimation(rbToLoad);
			}
		}
	}

	/* Loading Blowers */
	const auto &blowersConfigToLoad = configMgr.getSceneBlowersConfig();

	std::vector<Storm::Vector3> areaVertexesTmp;
	std::vector<uint32_t> areaIndexesTmp;
	for (const Storm::SceneBlowerConfig &blowerToLoad : blowersConfigToLoad)
	{
		// Generate the mesh area data to pass to the graphical resources.
#define STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER(BlowerTypeName, BlowerTypeXmlName, EffectAreaType, MeshMakerType) \
case Storm::BlowerType::BlowerTypeName: \
	Storm::MeshMakerType::generate(blowerToLoad, areaVertexesTmp, areaIndexesTmp); \
	break;

		switch (blowerToLoad._blowerType)
		{
			STORM_XMACRO_GENERATE_BLOWERS_CODE;

		default:
		case Storm::BlowerType::None:
			Storm::throwException<Storm::Exception>("Unknown Blower to be created!");
			break;
		}

#undef STORM_XMACRO_GENERATE_ELEMENTARY_BLOWER

		threadMgr.executeOnThread(Storm::ThreadEnumeration::GraphicsThread, [&graphicsMgr, &blowerToLoad, areaVertexes = std::move(areaVertexesTmp), areaIndexes = std::move(areaIndexesTmp)]()
		{
			graphicsMgr.loadBlower(blowerToLoad, areaVertexes, areaIndexes);
		});

		simulMgr.loadBlower(blowerToLoad);
	}

	/* Set the physics time to what was saved in the state file if needed */
	if (shouldLoadState && loadedState._settings._loadPhysicsTime)
	{
		Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
		timeMgr.setCurrentPhysicsElapsedTime(loadedState._simulationState->_currentPhysicsTime);

		// Needed to set the blowers to the right state time.
		simulMgr.setBlowersStateTime(loadedState._simulationState->_currentPhysicsTime);
	}

	/* Load fluid particles */
	const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();
	if (sceneSimulationConfig._hasFluid)
	{
		LOG_COMMENT << "Loading fluid particles";

		const auto &fluidsConfigToLoad = configMgr.getSceneFluidConfig();
		if (shouldLoadState)
		{
			Storm::SimulationState &state = *loadedState._simulationState;

			const auto endStatesIter = std::end(state._pSystemStates);
			if (auto fluidStateFound = std::find_if(std::begin(state._pSystemStates), endStatesIter, [id = fluidsConfigToLoad._fluidId](const Storm::SystemSimulationStateObject &fluidState)
			{
				return fluidState._id == id;
			}); fluidStateFound != endStatesIter)
			{
				Storm::SystemSimulationStateObject &fluidState = *fluidStateFound;
				if (!fluidState._isFluid)
				{
					Storm::throwException<Storm::Exception>("The state " + std::to_string(fluidState._isFluid) + " we're trying to load as a fluid is in fact the state of a rigid body system! It isn't allowed!");
				}

				waitForFutures(asyncLoadingArray);
				this->removeRbInsiderFluidParticle(fluidState._positions, &fluidState);

				// We need to update the position to regenerate the position of any rigid body particle according to its translation.
				// This needs to be done only for rigid bodies. Fluids don't need it. 
				simulMgr.refreshParticlesPosition();

				simulMgr.addFluidParticleSystem(std::move(fluidState));
			}
			else
			{
				Storm::throwException<Storm::Exception>("We cannot find the fluid state with id matching the one in the configuration (" + std::to_string(fluidsConfigToLoad._fluidId) + ')');
			}
		}
		else
		{
			const float particleRadius = sceneSimulationConfig._particleRadius;
			const float particleDiameter = particleRadius * 2.f;
			std::vector<Storm::Vector3> fluidParticlePos;

			// First, evaluate the particle total count we need to have (to avoid unneeded reallocations along the way)...
			fluidParticlePos.reserve(std::accumulate(std::begin(fluidsConfigToLoad._fluidGenConfig), std::end(fluidsConfigToLoad._fluidGenConfig), static_cast<std::size_t>(0),
				[particleDiameter](const std::size_t accumulatedVal, const Storm::SceneFluidBlockConfig &fluidBlockGenerated)
			{
				std::size_t particleXCount;
				std::size_t particleYCount;
				std::size_t particleZCount;

				switch (fluidBlockGenerated._loadDenseMode)
				{
#define STORM_EXECUTE_METHOD_ON_DENSE_MODE(DenseMode) computeParticleCountBoxExtents<DenseMode>(fluidBlockGenerated, particleDiameter, particleXCount, particleYCount, particleZCount);

					STORM_EXECUTE_CASE_ON_DENSE_MODE(FluidParticleLoadDenseMode::Normal);
					STORM_EXECUTE_CASE_ON_DENSE_MODE(FluidParticleLoadDenseMode::AsSplishSplash);

#undef STORM_EXECUTE_METHOD_ON_DENSE_MODE

				default:
					break;
				}

				return accumulatedVal + particleXCount * particleYCount * particleZCount;
			}) + fluidsConfigToLoad._fluidUnitParticleGenConfig.size());

			for (const Storm::SceneFluidBlockConfig &fluidBlockGenerated : fluidsConfigToLoad._fluidGenConfig)
			{
				switch (fluidBlockGenerated._loadDenseMode)
				{
#define STORM_EXECUTE_METHOD_ON_DENSE_MODE(DenseMode) generateFluidParticles<DenseMode>(fluidParticlePos, fluidBlockGenerated, particleDiameter);

					STORM_EXECUTE_CASE_ON_DENSE_MODE(FluidParticleLoadDenseMode::Normal);
					STORM_EXECUTE_CASE_ON_DENSE_MODE(FluidParticleLoadDenseMode::AsSplishSplash);

#undef STORM_EXECUTE_METHOD_ON_DENSE_MODE

				default:
					break;
				}
			}

			for (const Storm::SceneFluidUnitParticleConfig &fluidUnitPToGenerate : fluidsConfigToLoad._fluidUnitParticleGenConfig)
			{
				fluidParticlePos.push_back(fluidUnitPToGenerate._position);
			}

			waitForFutures(asyncLoadingArray);

			this->removeRbInsiderFluidParticle(fluidParticlePos, nullptr);

			// We need to update the position to regenerate the position of any rigid body particle according to its translation.
			// This needs to be done only for rigid bodies. Fluids don't need it. 
			simulMgr.refreshParticlesPosition();

			simulMgr.addFluidParticleSystem(fluidsConfigToLoad._fluidId, std::move(fluidParticlePos));
		}
	}
	else
	{
		LOG_WARNING <<
			"No fluid was present and we allowed it... Therefore, we would skip fluid loading to concentrate on rigid body interactions.\n"
			"It is a debug/development helper feature so don't let this setting remains because this isn't the purpose of the application...";

		waitForFutures(asyncLoadingArray);

		simulMgr.refreshParticlesPosition();
	}

	/* Load constraints */
	const auto &constraintsConfig = configMgr.getSceneConstraintsConfig();
	if (!constraintsConfig.empty())
	{
		LOG_COMMENT << "Loading Constraints";
		physicsMgr.loadConstraints(constraintsConfig);
	}
	else
	{
		LOG_DEBUG << "No Constraints to load. Skipping this step.";
	}
}

const std::vector<std::shared_ptr<Storm::IRigidBody>>& Storm::AssetLoaderManager::getRigidBodyArray() const
{
	return _rigidBodies;
}

void Storm::AssetLoaderManager::generateSimpleSmoothedCube(const Storm::Vector3 &position, const Storm::Vector3 &dimension, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals /*= nullptr*/) const
{
	Storm::BasicMeshGenerator::generateSmoothedCube(position, dimension, inOutVertexes, inOutIndexes, inOutNormals);
}

void Storm::AssetLoaderManager::generateSimpleSphere(const Storm::Vector3 &position, const float radius, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals /*= nullptr*/) const
{
	Storm::BasicMeshGenerator::generateSphere(position, radius, inOutVertexes, inOutIndexes, inOutNormals);
}

void Storm::AssetLoaderManager::generateSimpleCylinder(const Storm::Vector3 &position, const float radius, const float height, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals /*= nullptr*/) const
{
	Storm::BasicMeshGenerator::generateCylinder(position, radius, height, inOutVertexes, inOutIndexes, inOutNormals);
}

void Storm::AssetLoaderManager::generateSimpleCone(const Storm::Vector3 &position, const float upRadius, const float downRadius, const float height, std::vector<Storm::Vector3> &inOutVertexes, std::vector<uint32_t> &inOutIndexes, std::vector<Storm::Vector3>*const inOutNormals /*= nullptr*/) const
{
	Storm::BasicMeshGenerator::generateCone(position, upRadius, downRadius, height, inOutVertexes, inOutIndexes, inOutNormals);
}

std::shared_ptr<Storm::AssetCacheData> Storm::AssetLoaderManager::retrieveAssetData(const Storm::AssetCacheDataOrder &order)
{
	std::unique_lock<std::mutex> generalLock{ _assetGeneralMutex };
	std::mutex &specificAssetMutex = _assetSpecificMutexMap[order._rbConfig._meshFilePath];
	auto &assetCacheDataArray = _cachedAssetData[order._rbConfig._meshFilePath];
	generalLock.unlock();

	std::lock_guard<std::mutex> lock{ specificAssetMutex };
	if (!assetCacheDataArray.empty())
	{
		for (const std::shared_ptr<Storm::AssetCacheData> &assetCacheDataPtr : assetCacheDataArray)
		{
			if (assetCacheDataPtr->isEquivalentWith(order._rbConfig, order._considerFinalInEquivalence))
			{
				return assetCacheDataPtr;
			}
		}

		// We already created a source so we can skip the mesh reimporting.
		return assetCacheDataArray.emplace_back(std::make_shared<Storm::AssetCacheData>(order._rbConfig, *assetCacheDataArray.front(), order._layerDistance));
	}

	switch (order._rbConfig._collisionShape)
	{
	case Storm::CollisionType::IndividualParticle:
		return assetCacheDataArray.emplace_back(std::make_shared<Storm::AssetCacheData>(order._rbConfig, order._assimpScene, order._layerDistance));

	case Storm::CollisionType::Cube:
	case Storm::CollisionType::Sphere:
	case Storm::CollisionType::None:
	case Storm::CollisionType::Custom:
	default:
		if (order._assimpScene)
		{
			return assetCacheDataArray.emplace_back(std::make_shared<Storm::AssetCacheData>(order._rbConfig, order._assimpScene, order._layerDistance));
		}
		else
		{
			return nullptr;
		}
	}
}

void Storm::AssetLoaderManager::clearCachedAssetData()
{
	_cachedAssetData.clear();
}

void Storm::AssetLoaderManager::removeRbInsiderFluidParticle(std::vector<Storm::Vector3> &inOutFluidParticles, Storm::SystemSimulationStateObject* inOutSimulStateObjectPtr) const
{
	for (const auto &cachedDataArrayPair : _cachedAssetData)
	{
		for (const auto &rbCachedDataPtr : cachedDataArrayPair.second)
		{
			rbCachedDataPtr->removeInsiderParticle(inOutFluidParticles, inOutSimulStateObjectPtr);
		}
	}
}

std::mutex& Storm::AssetLoaderManager::getAddingMutex() const
{
	return _addingMutex;
}

std::mutex& Storm::AssetLoaderManager::getAssetMutex(const std::string &assetUID, bool &outMutexExistedBefore)
{
	outMutexExistedBefore = true;

	std::lock_guard<std::mutex> lock{ _assetGeneralMutex };
	if (auto found = _assetSpecificMutexMap.find(assetUID); found != std::end(_assetSpecificMutexMap))
	{
		return found->second;
	}
	else
	{
		outMutexExistedBefore = false;
		return _assetSpecificMutexMap[assetUID];
	}
}
