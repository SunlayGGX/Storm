#include "EmitterObject.h"

#include "SceneSmokeEmitterConfig.h"

#include "IRandomManager.h"
#include "ISimulatorManager.h"
#include "SingletonHolder.h"

#include "PushedParticleEmitterData.h"

#include "ThreadingSafety.h"
#include "RunnerHelper.h"



Storm::EmitParticleData::EmitParticleData(const Storm::Vector3 &position, float time) :
	_position{ position },
	_remainingTime{ time }
{

}

Storm::EmitterObject::EmitterObject(const SceneSmokeEmitterConfig &associatedCfg) :
	_enabled{ true },
	_cfg{ associatedCfg },
	_spawningTime{ 1.f / _cfg._emitCountPerSeconds },
	_nextSpawnTime{ 0.f },
	_currentEmitterTime{ 0.f }
{

}

void Storm::EmitterObject::update(float deltaTime, Storm::PushedParticleEmitterData &appendDataThisFrame)
{
	assert(Storm::isSimulationThread() && "This method should only be used in simulation thread!");
	assert(this->isEnabled() && "This method should be used after testing against enabling flag.");

	appendDataThisFrame._id = _cfg._emitterId;

	this->decreaseEmittedLife(deltaTime);
	this->updateEmittedList(deltaTime);
	this->emitNew(deltaTime);
	_currentEmitterTime += deltaTime;

	for (auto &emitted : _emitted)
	{
		appendDataThisFrame._positions.emplace_back(emitted._position);
	}
}

void Storm::EmitterObject::updateEmittedList(float deltaTime)
{
	Storm::runParallel(_emitted, [this, deltaTime](auto &emitted)
	{
		this->updateEmittedData(deltaTime, emitted);
	});
}

void Storm::EmitterObject::updateEmittedData(float deltaTime, EmitParticleData &emitted) const
{
	const auto &simulatorMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ISimulatorManager>();
	const Storm::Vector3 interpolatedVelocity = simulatorMgr.interpolateVelocityAtPosition(emitted._position);

	emitted._position += interpolatedVelocity * deltaTime;
}

void Storm::EmitterObject::decreaseEmittedLife(float deltaTime)
{
	for (auto &emitted : _emitted)
	{
		emitted._remainingTime -= deltaTime;
	}

	_emitted.remove_if([](const auto &particle)
	{
		return particle._remainingTime <= 0.f;
	});
}

void Storm::EmitterObject::emitNew(float deltaTime)
{
	if (_currentEmitterTime + deltaTime < _nextSpawnTime)
	{
		return;
	}

	const float lastSpawnTime = _nextSpawnTime - _spawningTime;
	const float deltaTimeSinceLastSpawn = _currentEmitterTime - lastSpawnTime + deltaTime;

	Storm::IRandomManager &randomMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IRandomManager>();

	const Storm::Vector3 deltaDisplacment{ 0.001f, 0.001f, 0.001f };

	std::size_t toSpawnCount = static_cast<std::size_t>(deltaTimeSinceLastSpawn / _spawningTime);
	_nextSpawnTime += static_cast<float>(toSpawnCount + 1) * _spawningTime;

	while (toSpawnCount != 0)
	{
		auto &newEmitted = _emitted.emplace_back(_cfg._position, _cfg._smokeAliveTimeSeconds);
		newEmitted._position += randomMgr.randomizeVector3(deltaDisplacment);
		--toSpawnCount;
	}
}
