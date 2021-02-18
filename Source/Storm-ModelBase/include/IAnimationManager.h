#pragma once


#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	struct SceneRigidBodyConfig;

	class IAnimationManager : public Storm::ISingletonHeldInterface<Storm::IAnimationManager>
	{
	public:
		virtual ~IAnimationManager() = default;

	public:
		virtual void createAnimation(const Storm::SceneRigidBodyConfig &rbConfig) = 0;

	public:
		virtual bool retrieveAnimationData(const float currentTime, const unsigned int rbId, Storm::Vector3 &outPos, Storm::Rotation &outRot) = 0;
	};
}
