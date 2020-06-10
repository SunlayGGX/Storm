#include "ParticleSystem.h"


namespace
{
	float computeDefaultParticleMass()
	{
		// TODO
		return 1.f;
	}
}


Storm::ParticleSystem::ParticleSystem(std::vector<Storm::Vector3> &&worldPositions) :
	_positions{ std::move(worldPositions) }
{
	const std::size_t particleCount = _positions.size();

	_masses.resize(particleCount, computeDefaultParticleMass());
	_velocity.resize(particleCount, Storm::Vector3::Zero());
	_accelerations.resize(particleCount);
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

void Storm::ParticleSystem::initializeIteration()
{

}

void Storm::ParticleSystem::updatePosition(float deltaTimeInSec)
{
	std::for_each(std::execution::par_unseq, std::begin(_accelerations), std::end(_accelerations), [this, deltaTimeInSec](const Storm::Vector3 &currentAccel)
	{
		const std::size_t currentParticleIndex = &currentAccel - &_accelerations[0];

		Storm::Vector3 &currentVelocity = _velocity[currentParticleIndex];
		currentVelocity += deltaTimeInSec * currentAccel;
		_positions[currentParticleIndex] += deltaTimeInSec * currentVelocity;
	});
}
