#pragma once


#include "Singleton.h"
#include "IAnimationManager.h"

#include "SingletonDefaultImplementation.h"


namespace Storm
{
	class AnimationObject;

	class AnimationManager final :
		private Storm::Singleton<Storm::AnimationManager, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::IAnimationManager
	{
		STORM_DECLARE_SINGLETON(AnimationManager);

	public:
		void createAnimation(const Storm::SceneRigidBodyConfig &rbConfig) final override;

		bool retrieveAnimationData(const float currentTime, const unsigned int rbId, Storm::Vector3 &outPos, Storm::Vector3 &outRot) final override;

	private:
		std::map<unsigned int, std::unique_ptr<Storm::AnimationObject>> _animationMap;
	};
}
