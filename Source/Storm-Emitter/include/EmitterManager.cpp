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
	if (std::size_t emittersCount = _emitters.size();
		emittersCount > 0)
	{
		std::vector<Storm::PushedParticleEmitterData> data;
		data.reserve(emittersCount);

		for (auto &emitter : _emitters)
		{
			if (emitter.isEnabled())
			{
				auto &newData = data.emplace_back();
				emitter.update(deltaTime, newData);
			}
		}

		if (!data.empty())
		{
			Storm::IGraphicsManager &graphicMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>();
			graphicMgr.pushSmokeEmittedData(std::move(data));
		}
	}
}
