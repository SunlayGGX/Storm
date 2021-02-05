#include "ParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "SimulatorManager.h"

#include "SceneSimulationConfig.h"
#include "SceneFluidConfig.h"

#include "RunnerHelper.h"

#include "MacroConfig.h"

#include "ThreadingSafety.h"



Storm::ParticleSystem::ParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions) :
	_positions{ std::move(worldPositions) },
	_particleSystemIndex{ particleSystemIndex },
	_isDirty{ true }
{
	this->initParticlesCount(_positions.size());
}

Storm::ParticleSystem::ParticleSystem(unsigned int particleSystemIndex, const std::size_t particleCount) :
	_particleSystemIndex{ particleSystemIndex },
	_isDirty{ true }
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	if (!configMgr.isInReplayMode())
	{
		Storm::throwException<Storm::Exception>(__FUNCSIG__ " is to be used only in replay mode!");
	}

	this->resizeParticlesCount(particleCount);
}

void Storm::ParticleSystem::initParticlesCount(const std::size_t particleCount)
{
	_velocity.resize(particleCount, Storm::Vector3::Zero());
	_force.resize(particleCount, Storm::Vector3::Zero());
	_tmpPressureForce.resize(particleCount, Storm::Vector3::Zero());
	_tmpViscosityForce.resize(particleCount, Storm::Vector3::Zero());

	const bool replayMode = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().isInReplayMode();
	if (!replayMode)
	{
		_neighborhood.resize(particleCount);

		for (auto &neighborHoodArray : _neighborhood)
		{
			neighborHoodArray.reserve(64);
		}
	}
}

void Storm::ParticleSystem::resizeParticlesCount(const std::size_t particleCount)
{
	_positions.resize(particleCount);
	this->initParticlesCount(particleCount);
}

std::vector<Storm::Vector3>& Storm::ParticleSystem::getPositions() noexcept
{
	return _positions;
}

const std::vector<Storm::Vector3>& Storm::ParticleSystem::getPositions() const noexcept
{
	return _positions;
}

const std::vector<Storm::Vector3>& Storm::ParticleSystem::getVelocity() const noexcept
{
	return _velocity;
}

std::vector<Storm::Vector3>& Storm::ParticleSystem::getVelocity() noexcept
{
	return _velocity;
}

const std::vector<Storm::Vector3>& Storm::ParticleSystem::getForces() const noexcept
{
	return _force;
}

const std::vector<Storm::Vector3>& Storm::ParticleSystem::getTemporaryPressureForces() const noexcept
{
	return _tmpPressureForce;
}

std::vector<Storm::Vector3>& Storm::ParticleSystem::getTemporaryPressureForces() noexcept
{
	return _tmpPressureForce;
}

const std::vector<Storm::Vector3>& Storm::ParticleSystem::getTemporaryViscosityForces() const noexcept
{
	return _tmpViscosityForce;
}

std::vector<Storm::Vector3>& Storm::ParticleSystem::getTemporaryViscosityForces() noexcept
{
	return _tmpViscosityForce;
}

std::vector<Storm::Vector3>& Storm::ParticleSystem::getForces() noexcept
{
	return _force;
}

const std::vector<Storm::ParticleNeighborhoodArray>& Storm::ParticleSystem::getNeighborhoodArrays() const noexcept
{
	return _neighborhood;
}

std::vector<Storm::ParticleNeighborhoodArray>& Storm::ParticleSystem::getNeighborhoodArrays() noexcept
{
	return _neighborhood;
}

std::size_t Storm::ParticleSystem::getParticleCount() const noexcept
{
	return _positions.size();
}

unsigned int Storm::ParticleSystem::getId() const noexcept
{
	return _particleSystemIndex;
}

bool Storm::ParticleSystem::isDirty() const noexcept
{
	return _isDirty;
}

void Storm::ParticleSystem::setIsDirty(bool dirty)
{
	_isDirty = dirty;
}

void Storm::ParticleSystem::prepareSaving(const bool replayMode)
{

}

void Storm::ParticleSystem::buildNeighborhood(const Storm::ParticleSystemContainer &allParticleSystems)
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");

	// First, clear all neighborhood
	Storm::runParallel(_neighborhood, [](Storm::ParticleNeighborhoodArray &neighbors)
	{
		neighbors.clear();
	});

	// Then fill them again with the right data
	const float kernelLength = Storm::SimulatorManager::instance().getKernelLength();
	this->buildNeighborhoodOnParticleSystemUsingSpacePartition(allParticleSystems, kernelLength);
}

void Storm::ParticleSystem::initializePreSimulation(const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLengthSquared)
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");
}

void Storm::ParticleSystem::onIterationStart()
{
#if defined(DEBUG) || defined(_DEBUG)
	const std::size_t particleCount = this->getParticleCount();

	const bool replayMode = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().isInReplayMode();
	assert(
		particleCount == _positions.size() &&
		particleCount == _velocity.size() &&
		particleCount == _force.size() &&
		particleCount == _tmpPressureForce.size() &&
		particleCount == _tmpViscosityForce.size() &&
		(replayMode || particleCount == _neighborhood.size()) &&
		"Particle count mismatch detected! An array of particle property has not the same particle count than the other!"
	);

	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");
#endif
}

void Storm::ParticleSystem::onSubIterationStart(const Storm::ParticleSystemContainer &, const std::vector<std::unique_ptr<Storm::IBlower>> &)
{
	assert(Storm::isSimulationThread() && "This method should only be executed inside the simulation thread!");
	_isDirty = false;
}

float Storm::ParticleSystem::computeParticleDefaultVolume()
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const float particleDiameter = 2.f * configMgr.getSceneSimulationConfig()._particleRadius;
	return particleDiameter * particleDiameter * particleDiameter;
}

bool Storm::ParticleSystem::isElligibleNeighborParticle(const float kernelLengthSquared, const float normSquared)
{
	return normSquared > 0.000001f && normSquared < kernelLengthSquared;
}
