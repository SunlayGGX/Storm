#pragma once


#include "Singleton.h"
#include "IPhysicsManager.h"


namespace Storm
{
	class PhysicsManager :
		private Storm::Singleton<PhysicsManager>,
		public Storm::IPhysicsManager
	{
		STORM_DECLARE_SINGLETON(PhysicsManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();
	};
}
