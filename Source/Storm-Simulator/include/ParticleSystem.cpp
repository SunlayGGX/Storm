#include "ParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "SimulatorManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"



Storm::ParticleSystem::ParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions, float particleMass) :
	_positions{ std::move(worldPositions) },
	_particleSystemIndex{ particleSystemIndex },
	_massPerParticle{ particleMass },
	_isDirty{ true }
{
	const std::size_t particleCount = _positions.size();

	_densities.resize(particleCount, particleMass / computeParticleDefaultVolume());
	_pressures.resize(particleCount);
	_velocity.resize(particleCount, Storm::Vector3::Zero());
	_force.resize(particleCount);
	_neighborhood.resize(particleCount);

	for (auto &neighborHoodArray : _neighborhood)
	{
		neighborHoodArray.reserve(16);
	}
}

std::vector<float>& Storm::ParticleSystem::getDensities() noexcept
{
	return _densities;
}

const std::vector<float>& Storm::ParticleSystem::getDensities() const noexcept
{
	return _densities;
}

const std::vector<float>& Storm::ParticleSystem::getPressures() const noexcept
{
	return _pressures;
}

std::vector<float>& Storm::ParticleSystem::getPressures() noexcept
{
	return _pressures;
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

std::vector<Storm::Vector3>& Storm::ParticleSystem::getForces() noexcept
{
	return _force;
}

const std::vector<std::vector<Storm::NeighborParticleInfo>>& Storm::ParticleSystem::getNeighborhoodArrays() const noexcept
{
	return _neighborhood;
}

std::vector<std::vector<Storm::NeighborParticleInfo>>& Storm::ParticleSystem::getNeighborhoodArrays() noexcept
{
	return _neighborhood;
}

float Storm::ParticleSystem::getMassPerParticle() const noexcept
{
	return _massPerParticle;
}

float Storm::ParticleSystem::getRestDensity() const noexcept
{
	return _restDensity;
}

unsigned int Storm::ParticleSystem::getId() const noexcept
{
	return _particleSystemIndex;
}

bool Storm::ParticleSystem::isDirty() const noexcept
{
	return _isDirty;
}

void Storm::ParticleSystem::buildNeighborhood(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &allParticleSystems)
{
	const float kernelLength = Storm::SimulatorManager::instance().getKernelLength();
	const float kernelLengthSquared = kernelLength * kernelLength;

	std::for_each(std::begin(allParticleSystems), std::end(allParticleSystems), [this, kernelLengthSquared](const auto &particleSystem)
	{
		this->buildNeighborhoodOnParticleSystem(*particleSystem.second, kernelLengthSquared);
	});
}

void Storm::ParticleSystem::postApplySPH()
{

}

void Storm::ParticleSystem::initializeIteration()
{
	_isDirty = false;

#if defined(DEBUG) || defined(_DEBUG)
	const std::size_t particleCount = _densities.size();

	assert(
		particleCount == _pressures.size() &&
		particleCount == _positions.size() &&
		particleCount == _velocity.size() &&
		particleCount == _force.size() &&
		particleCount == _neighborhood.size() &&
		"Particle count mismatch detected! An array of particle property has not the same particle count than the other!"
	);
	
#endif
}

float Storm::ParticleSystem::computeParticleDefaultVolume()
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const float particleRadius = configMgr.getGeneralSimulationData()._particleRadius;
	return particleRadius * particleRadius * particleRadius;
}

bool Storm::ParticleSystem::isElligibleNeighborParticle(const float kernelLengthSquared, const float normSquared)
{
	return normSquared > 0.000000001f && normSquared < kernelLengthSquared;
}
