#include "ParticleSystem.h"



Storm::ParticleSystem::ParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions, float particleMass) :
	_positions{ std::move(worldPositions) },
	_particleSystemIndex{ particleSystemIndex }
{
	const std::size_t particleCount = _positions.size();

	_masses.resize(particleCount, particleMass);
	_velocity.resize(particleCount, Storm::Vector3::Zero());
	_accelerations.resize(particleCount);
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

bool Storm::ParticleSystem::isDirty() const noexcept
{
	return _isDirty;
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
