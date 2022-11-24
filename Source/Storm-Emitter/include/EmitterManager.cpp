#include "EmitterManager.h"
#include "EmitterObject.h"

#include "IGraphicsManager.h"
#include "IConfigManager.h"
#include "IThreadManager.h"
#include "SingletonHolder.h"

#include "SceneSmokeEmitterConfig.h"

#include "PushedParticleEmitterData.h"

#include "ThreadEnumeration.h"


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

void Storm::EmitterManager::setEmitterEnabled(unsigned int emitterID, bool enable)
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, emitterID, enable]()
	{
		if (auto found = std::find_if(std::begin(_emitters), std::end(_emitters), [emitterID](const Storm::EmitterObject &emitter)
		{
			return emitter.getID() == emitterID;
		}); found != std::end(_emitters))
		{
			found->setEnabled(enable);
		}
		else
		{
			Storm::throwException<Storm::Exception>("Emitter with id " + Storm::toStdString(emitterID) + " does not exist!");
		}
	});
}

void Storm::EmitterManager::setEmitterPauseEmission(unsigned int emitterID, bool pause)
{
	Storm::SingletonHolder::instance().getSingleton<Storm::IThreadManager>().executeOnThread(Storm::ThreadEnumeration::MainThread, [this, emitterID, pause]()
	{
		if (auto found = std::find_if(std::begin(_emitters), std::end(_emitters), [emitterID](const Storm::EmitterObject &emitter)
		{
			return emitter.getID() == emitterID;
		}); found != std::end(_emitters))
		{
			found->setEmissionPaused(pause);
		}
		else
		{
			Storm::throwException<Storm::Exception>("Emitter with id " + Storm::toStdString(emitterID) + " does not exist!");
		}
	});
}
