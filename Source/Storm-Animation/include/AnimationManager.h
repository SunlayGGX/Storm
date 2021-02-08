#pragma once


#include "Singleton.h"
#include "IAnimationManager.h"

#include "SingletonDefaultImplementation.h"


namespace Storm
{
	class AnimationManager final :
		private Storm::Singleton<Storm::AnimationManager, Storm::DefineDefaultInitAndCleanupImplementation>,
		public Storm::IAnimationManager
	{
		STORM_DECLARE_SINGLETON(AnimationManager);
	};
}
