#include "RigidBodyParticleSystem.h"

#include "SingletonHolder.h"
#include "IPhysicsManager.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"
#include "RigidBodySceneData.h"
#include "CollisionType.h"

#include "RunnerHelper.h"



namespace
{
	float computeDefaultRigidBodyParticleMass(unsigned int particleSystemIndex, const std::size_t particleCount)
	{
		const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

		const Storm::RigidBodySceneData &currentRbData = configMgr.getRigidBodyData(particleSystemIndex);
		return currentRbData._mass / static_cast<float>(particleCount);
	}
}

Storm::RigidBodyParticleSystem::RigidBodyParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions) :
	Storm::ParticleSystem{ particleSystemIndex, std::move(worldPositions), computeDefaultRigidBodyParticleMass(particleSystemIndex, worldPositions.size()) },
	_cachedTrackedRbRotationQuat{ Storm::Quaternion::Identity() }
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();
	const float k_particleRadius = generalSimulData._particleRadius;

	_restDensity = _massPerParticle / (k_particleRadius * k_particleRadius * k_particleRadius);

	const Storm::RigidBodySceneData &currentRbData = configMgr.getRigidBodyData(particleSystemIndex);

	_isWall = currentRbData._isWall;
	_isStatic = _isWall || currentRbData._static;
}

void Storm::RigidBodyParticleSystem::initializeIteration()
{
	Storm::ParticleSystem::initializeIteration();

	Storm::runParallel(_force, [](Storm::Vector3 &force)
	{
		force.setZero();
	});
}

const std::vector<float>& Storm::RigidBodyParticleSystem::getPredictedDensities() const
{
	Storm::throwException<std::logic_error>(__FUNCTION__ " shouldn't be called for rigid bodies particle system (only implemented for fluids)");
}

std::vector<float>& Storm::RigidBodyParticleSystem::getPredictedDensities()
{
	Storm::throwException<std::logic_error>(__FUNCTION__ " shouldn't be called for rigid bodies particle system (only implemented for fluids)");
}

const std::vector<Storm::Vector3>& Storm::RigidBodyParticleSystem::getPredictedPressureForces() const
{
	Storm::throwException<std::logic_error>(__FUNCTION__ " shouldn't be called for rigid bodies particle system (only implemented for fluids)");
}

std::vector<Storm::Vector3>& Storm::RigidBodyParticleSystem::getPredictedPressureForces()
{
	Storm::throwException<std::logic_error>(__FUNCTION__ " shouldn't be called for rigid bodies particle system (only implemented for fluids)");
}

const std::vector<Storm::Vector3>& Storm::RigidBodyParticleSystem::getPredictedPositions() const
{
	Storm::throwException<std::logic_error>(__FUNCTION__ " shouldn't be called for rigid bodies particle system (only implemented for fluids)");
}

std::vector<Storm::Vector3>& Storm::RigidBodyParticleSystem::getPredictedPositions()
{
	Storm::throwException<std::logic_error>(__FUNCTION__ " shouldn't be called for rigid bodies particle system (only implemented for fluids)");
}

bool Storm::RigidBodyParticleSystem::isFluids() const noexcept
{
	return false;
}

bool Storm::RigidBodyParticleSystem::isStatic() const noexcept
{
	return _isStatic;
}

bool Storm::RigidBodyParticleSystem::isWall() const noexcept
{
	return _isWall;
}

void Storm::RigidBodyParticleSystem::buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared)
{
	if (otherParticleSystem.isFluids())
	{
		Storm::runParallel(_positions, [this, kernelLengthSquared, &otherParticleSystem](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
		{
			std::vector<Storm::NeighborParticleInfo> &currentNeighborhoodToFill = _neighborhood[currentPIndex];
			currentNeighborhoodToFill.clear();

			const auto &otherParticleSystemPositionsArray = otherParticleSystem.getPositions();
			const std::size_t otherParticleCount = otherParticleSystemPositionsArray.size();

			for (std::size_t particleIndex = 0; particleIndex < otherParticleCount; ++particleIndex)
			{
				const Storm::Vector3 positionDifference = currentPPosition - otherParticleSystemPositionsArray[particleIndex];
				const float vectToParticleSquaredNorm = positionDifference.squaredNorm();
				if (Storm::ParticleSystem::isElligibleNeighborParticle(kernelLengthSquared, vectToParticleSquaredNorm))
				{
					currentNeighborhoodToFill.emplace_back(const_cast<Storm::ParticleSystem*>(&otherParticleSystem), particleIndex, positionDifference, vectToParticleSquaredNorm, true);
				}
			}
		});
	}
}

void Storm::RigidBodyParticleSystem::updatePosition(float deltaTimeInSec)
{
	Storm::Vector3 currentPosition;
	Storm::Quaternion currentQuatRotation;
	Storm::SingletonHolder::instance().getSingleton<Storm::IPhysicsManager>().getMeshTransform(_particleSystemIndex, currentPosition, currentQuatRotation);

	if (currentPosition != _cachedTrackedRbPosition || currentQuatRotation.x() != _cachedTrackedRbRotationQuat.x() || currentQuatRotation.y() != _cachedTrackedRbRotationQuat.y() || currentQuatRotation.z() != _cachedTrackedRbRotationQuat.z() || currentQuatRotation.w() != _cachedTrackedRbRotationQuat.w())
	{
		_isDirty = true;

		const Storm::Quaternion oldRbRotationConjugateQuat = _cachedTrackedRbRotationQuat.conjugate();
		const Storm::Quaternion currentConjugateQuatRotation = currentQuatRotation.conjugate();

		Storm::runParallel(_positions, [&](Storm::Vector3 &currentParticlePosition)
		{
			/* First, remove the current world space coordinate to revert to the object space coordinate (where particle were defined initially and on which PhysX was initialized). */

			// 1- Remove the translation
			Storm::Quaternion currentPosAsPureQuat{ 0.f, currentParticlePosition.x() - _cachedTrackedRbPosition.x(), currentParticlePosition.y() - _cachedTrackedRbPosition.y(), currentParticlePosition.z() - _cachedTrackedRbPosition.z() };

			// 2- Remove the rotation
			currentPosAsPureQuat = oldRbRotationConjugateQuat * currentPosAsPureQuat * _cachedTrackedRbRotationQuat;


			/* Finally, once we're in object space coordinate, reapply the correct transformation to go back to the world coordinate (with correct transformation) */

			// 1- Apply the rotation
			currentPosAsPureQuat = currentQuatRotation * currentPosAsPureQuat * currentConjugateQuatRotation;

			// 2- Apply the translation
			currentParticlePosition = currentPosAsPureQuat.vec() + currentPosition;
		});

		_cachedTrackedRbPosition = currentPosition;
		_cachedTrackedRbRotationQuat = currentQuatRotation;
	}
}

void Storm::RigidBodyParticleSystem::postApplySPH()
{
	Storm::IPhysicsManager &physicMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IPhysicsManager>();
	physicMgr.applyLocalForces(_particleSystemIndex, _positions, _force);
}

void Storm::RigidBodyParticleSystem::flushPressureToTotalForce()
{
	// Nothing to do, rigid body particle system doesn't have predicted pressure.
}
