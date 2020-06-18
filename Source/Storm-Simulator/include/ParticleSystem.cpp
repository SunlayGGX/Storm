#include "ParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"

#include "SimulationMode.h"



Storm::ParticleSystem::ParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions, float particleMass) :
	_positions{ std::move(worldPositions) },
	_particleSystemIndex{ particleSystemIndex }
{
	const std::size_t particleCount = _positions.size();

	_masses.resize(particleCount, particleMass);
	_velocity.resize(particleCount, Storm::Vector3::Zero());
	_accelerations.resize(particleCount);
	_neighborhood.resize(particleCount);

	for (auto &neighborHoodArray : _neighborhood)
	{
		neighborHoodArray.reserve(16);
	}
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

	std::for_each(std::begin(allParticleSystems), std::end(allParticleSystems), [this, kernelLengthSquared](const auto &particleSystem)
	{
		this->buildNeighborhoodOnParticleSystem(*particleSystem.second, kernelLengthSquared);
	});
}

void Storm::ParticleSystem::executeSPH(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &allParticleSystems)
{
	const Storm::GeneralSimulationData &generalSimulData = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getGeneralSimulationData();
	switch (generalSimulData._simulationMode)
	{
	case Storm::SimulationMode::PCISPH:
		this->executePCISPH();
		break;
	}
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
