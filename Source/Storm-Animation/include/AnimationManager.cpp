#include "AnimationManager.h"

#include "AnimationObject.h"

#include "SceneRigidBodyConfig.h"

#include "ThreadingSafety.h"



Storm::AnimationManager::AnimationManager() = default;
Storm::AnimationManager::~AnimationManager() = default;

void Storm::AnimationManager::createAnimation(const Storm::SceneRigidBodyConfig &rbConfig)
{
	if (rbConfig._animationXmlContent.empty())
	{
		LOG_WARNING << "Animation creation ignored for rigid body " << rbConfig._rigidBodyID << " because animation content is empty!";
		return;
	}

	assert(Storm::isAnimationThread() && "This method should only be called from animation thread!");

	if (const auto found = _animationMap.find(rbConfig._rigidBodyID); found == std::end(_animationMap))
	{
		const auto addedAnimIter = _animationMap.emplace_hint(found, rbConfig._rigidBodyID, std::make_unique<Storm::AnimationObject>(rbConfig));
		LOG_DEBUG << "Animation created from rigid body " << rbConfig._rigidBodyID << " with " << addedAnimIter->second->getKeyframeCount() << " keyframes.";
	}
	else
	{
		Storm::throwException<Storm::Exception>("Cannot create another animation bound to the same rigid body (rigid body " + std::to_string(rbConfig._rigidBodyID) + ")");
	}
}

bool Storm::AnimationManager::retrieveAnimationData(const float currentTime, const unsigned int rbId, Storm::Vector3 &outPos, Storm::Vector3 &outRot)
{
	assert(Storm::isAnimationThread() && "This method should only be called from animation thread!");

	if (const auto found = _animationMap.find(rbId); found != std::end(_animationMap))
	{
		if (!found->second->getCurrentFrameOrShouldBeDestroyed(currentTime, outPos, outRot))
		{
			_animationMap.erase(found);
			return false;
		}

		return true;
	}
	else
	{
		Storm::throwException<Storm::Exception>("Animation for rigid body " + std::to_string(rbId) + " not found!");
	}
}
