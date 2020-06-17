#include "ParticleSystem.h"

#include "ParticleIdentifier.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"



Storm::ParticleSystem::ParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions, float particleMass) :
	_positions{ std::move(worldPositions) },
	_particleSystemIndex{ particleSystemIndex }
{
	const std::size_t particleCount = _positions.size();

	_masses.resize(particleCount, particleMass);
	_velocity.resize(particleCount, Storm::Vector3::Zero());
	_accelerations.resize(particleCount);
	_neighborhood.resize(particleCount);
}

const std::vector<float>& Storm::ParticleSystem::getMasses() const noexcept
{
	return _masses;
}

const std::vector<Storm::Vector3>& Storm::ParticleSystem::getPositions() const noexcept
{
	return _positions;
}

const std::vector<Storm::Vector3>& Storm::ParticleSystem::getVelocity() const noexcept
{
	return _velocity;
}

const std::vector<Storm::Vector3>& Storm::ParticleSystem::getAccelerations() const noexcept
{
	return _accelerations;
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
	const Storm::GeneralSimulationData &generalSimulData = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getGeneralSimulationData();

	const float kernelLength = generalSimulData._kernelCoefficient * generalSimulData._particleRadius;
	const float kernelLengthSquared = kernelLength * kernelLength;

	std::for_each(std::execution::par_unseq, std::begin(allParticleSystems), std::end(allParticleSystems), [this, kernelLengthSquared](const auto &particleSystem)
	{
		this->buildNeighborhoodOnParticleSystem(*particleSystem.second, kernelLengthSquared);
	});
}

void Storm::ParticleSystem::initializeIteration()
{
	_isDirty = false;

	assert(
		_masses.size() == _positions.size() &&
		_masses.size() == _velocity.size() &&
		_masses.size() == _accelerations.size() &&
		"Particle count mismatch detected! An array of particle property has not the same particle count than the other!"
	);
}

void Storm::ParticleSystem::addNeighborIfRelevant(std::vector<Storm::ParticleIdentifier> &currentNeighborhoodToFill, const Storm::Vector3 &currentParticlePosition, const Storm::Vector3 &maybeNeighborhood, unsigned int particleSystemIndex, std::size_t particleIndex, const float kernelLengthSquared)
{
	if ((currentParticlePosition - maybeNeighborhood).squaredNorm() < kernelLengthSquared)
	{
		currentNeighborhoodToFill.emplace_back(particleSystemIndex, particleIndex);
	}
}
