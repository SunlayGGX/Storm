#include "EmitterManager.h"
#include "EmitterObject.h"

#include "IGraphicsManager.h"
#include "IConfigManager.h"
#include "SingletonHolder.h"

#include "SceneSmokeEmitterConfig.h"

#include "PushedParticleEmitterData.h"


Storm::EmitterManager::EmitterManager() = default;
Storm::EmitterManager::~EmitterManager() = default;

void Storm::EmitterManager::initialize_Implementation()
{
	const auto &emitterCfgs = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getSceneSmokeEmittersConfig();
	for (const auto &emitterCfg : emitterCfgs)
	{
		_emitters.emplace_back(emitterCfg);
	}
}

void Storm::EmitterManager::cleanUp_Implementation()
{
	_emitters.clear();
}

void Storm::EmitterManager::update(float deltaTime)
{
	// we start at _emitters.size() because maybe each emitter would spawn 1 smoke particle. It's just an heuristic to not realloc all the time.
	if (std::size_t expectedCount = _emitters.size();
		expectedCount > 0)
	{
		Storm::PushedParticleEmitterData data;

		for (const auto &emitter : _emitters)
		{
			expectedCount += emitter.getEmittedCount();
		}
		data._positions.reserve(expectedCount);

		for (auto &emitter : _emitters)
		{
			emitter.update(deltaTime, data);
		}

		if (!data._positions.empty())
		{
			Storm::IGraphicsManager &graphicMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>();
			graphicMgr.pushSmokeEmittedData(std::move(data));
		}
	}
}
