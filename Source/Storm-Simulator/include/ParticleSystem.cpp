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
