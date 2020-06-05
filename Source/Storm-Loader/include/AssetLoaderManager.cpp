#include "AssetLoaderManager.h"

#include "AssimpLoggingWrapper.h"

#include "SingletonHolder.h"
#include "ITimeManager.h"
#include "IConfigManager.h"
#include "IGraphicsManager.h"

#include "SceneData.h"
#include "RigidBodySceneData.h"
#include "RigidBody.h"

#include <Assimp\DefaultLogger.hpp>


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
}



Storm::AssetLoaderManager::AssetLoaderManager() = default;
Storm::AssetLoaderManager::~AssetLoaderManager() = default;

void Storm::AssetLoaderManager::initialize_Implementation()
{
	initializeAssimpLogger();

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const auto &rigidBodiesDataToLoad = configMgr.getSceneData()._rigidBodiesData;

	std::filesystem::create_directories(Storm::RigidBody::retrieveParticleDataCacheFolder());

	Storm::IGraphicsManager &graphicsMgr = singletonHolder.getSingleton<Storm::IGraphicsManager>();

	_rigidBodies.reserve(rigidBodiesDataToLoad.size());
	for (const auto &rbToLoad : rigidBodiesDataToLoad)
	{
		auto &emplacedRb = _rigidBodies.emplace_back(std::static_pointer_cast<Storm::IRigidBody>(std::make_shared<Storm::RigidBody>(rbToLoad)));
		const unsigned int emplacedRbId = emplacedRb->getRigidBodyID();

		graphicsMgr.bindParentRbToMesh(emplacedRbId, emplacedRb);
	}
}

void Storm::AssetLoaderManager::cleanUp_Implementation()
{
	Assimp::DefaultLogger::kill();
}

const std::vector<std::shared_ptr<Storm::IRigidBody>>& Storm::AssetLoaderManager::getRigidBodyArray() const
{
	return _rigidBodies;
}
