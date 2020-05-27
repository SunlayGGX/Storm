#pragma once


#include "Singleton.h"
#include "IPhysicsManager.h"


namespace Storm
{
	class PhysXHandler;

	class PhysicsManager :
		private Storm::Singleton<PhysicsManager>,
		public Storm::IPhysicsManager
	{
		STORM_DECLARE_SINGLETON(PhysicsManager);

	private:
		void initialize_Implementation();
		void cleanUp_Implementation();

	private:
		std::unique_ptr<Storm::PhysXHandler> _physXHandler;
	};
}
