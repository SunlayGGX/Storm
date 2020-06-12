#include "RigidBodyParticleSystem.h"

#include "SingletonHolder.h"
#include "IPhysicsManager.h"


Storm::RigidBodyParticleSystem::RigidBodyParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions) :
	Storm::ParticleSystem{ particleSystemIndex, std::move(worldPositions) },
	_cachedTrackedRbRotationQuat{ Storm::Quaternion::Identity() }
{

}

bool Storm::RigidBodyParticleSystem::isFluids() const noexcept
{
	return false;
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
