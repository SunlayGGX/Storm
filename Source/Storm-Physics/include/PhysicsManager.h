#pragma once


#include "Singleton.h"
#include "IPhysicsManager.h"


namespace Storm
{
	class PhysXHandler;
	class PhysicsDynamicRigidBody;

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
		void bindParentRbToPhysicalBody(const Storm::RigidBodySceneData &rbSceneData, const std::shared_ptr<Storm::IRigidBody> &parentRb) final override;

	private:
		std::unique_ptr<Storm::PhysXHandler> _physXHandler;

		std::map<unsigned int, std::unique_ptr<Storm::PhysicsDynamicRigidBody>> _dynamicsRbMap;
	};
}
