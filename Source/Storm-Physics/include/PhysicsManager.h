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

	public:
		void addPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::vector<Storm::Vector3> &vertexes) final override;
		void bindParentRbToPhysicalBody(unsigned int meshId, const std::shared_ptr<Storm::IRigidBody> &parentRb) final override;

	private:
		std::unique_ptr<Storm::PhysXHandler> _physXHandler;

	};
}
