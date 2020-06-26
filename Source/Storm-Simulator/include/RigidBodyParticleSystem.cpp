#include "RigidBodyParticleSystem.h"

#include "SingletonHolder.h"
#include "IPhysicsManager.h"
#include "IConfigManager.h"

#include "RigidBodySceneData.h"
#include "CollisionType.h"



namespace
{
	float computeDefaultRigidBodyParticleMass(unsigned int particleSystemIndex, const std::size_t particleCount)
	{
		const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

		const auto &allRbData = configMgr.getRigidBodiesData();
		if (const auto currentRbDataIter = std::find_if(std::begin(allRbData), std::end(allRbData), [particleSystemIndex](const Storm::RigidBodySceneData &rb)
		{
			return rb._rigidBodyID == particleSystemIndex;
		}); currentRbDataIter != std::end(allRbData))
		{
			return currentRbDataIter->_mass / static_cast<float>(particleCount);
		}
		else
		{
			Storm::throwException<std::exception>("Cannot find rigid body data with index " + std::to_string(particleSystemIndex));
		}
	}
}

Storm::RigidBodyParticleSystem::RigidBodyParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions) :
	Storm::ParticleSystem{ particleSystemIndex, std::move(worldPositions), computeDefaultRigidBodyParticleMass(particleSystemIndex, worldPositions.size()) },
	_cachedTrackedRbRotationQuat{ Storm::Quaternion::Identity() }
{

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

void Storm::RigidBodyParticleSystem::buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared)
{
	if (otherParticleSystem.isFluids())
	{
		std::for_each(std::execution::par_unseq, std::begin(_positions), std::end(_positions), [this, kernelLengthSquared, &otherParticleSystem](const Storm::Vector3 &currentParticlePosition)
		{
			std::vector<Storm::NeighborParticleInfo> &currentNeighborhoodToFill = _neighborhood[this->getParticleIndex(_positions, currentParticlePosition)];
			currentNeighborhoodToFill.clear();

			const auto &otherParticleSystemPositionsArray = otherParticleSystem.getPositions();
			const std::size_t otherParticleSizeCount = otherParticleSystemPositionsArray.size();

			for (std::size_t particleIndex = 0; particleIndex < otherParticleSizeCount; ++particleIndex)
			{
				const Storm::Vector3 positionDifference = currentParticlePosition - otherParticleSystemPositionsArray[particleIndex];
				const float vectToParticleSquaredNorm = positionDifference.squaredNorm();
				if (vectToParticleSquaredNorm < kernelLengthSquared)
				{
					currentNeighborhoodToFill.emplace_back(&otherParticleSystem, particleIndex, positionDifference, vectToParticleSquaredNorm, true);
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

		std::for_each(std::execution::par_unseq, std::begin(_positions), std::end(_positions), [&](Storm::Vector3 &currentParticlePosition)
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

void Storm::RigidBodyParticleSystem::applyPredictedPressureToTotalForce()
{
	// Nothing to do, rigid body particle system doesn't have predicted pressure.
}
