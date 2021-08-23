#pragma once


namespace Storm
{
	struct ScenePhysicsConfig
	{
	public:
		ScenePhysicsConfig();

	public:
		bool _enablePCM; // Enable physx::PxSceneFlag::eENABLE_PCM
		bool _enableAdaptiveForce; // Enable physx::PxSceneFlag::eADAPTIVE_FORCE
		bool _enableFrictionEveryIteration; // Enable physx::PxSceneFlag::eENABLE_FRICTION_EVERY_ITERATION
		bool _enableStabilization; // Enable physx::PxSceneFlag::eENABLE_STABILIZATION
		bool _enableKinematicPairs; // Enable physx::PxSceneFlag::eENABLE_KINEMATIC_PAIRS
		bool _enableKinematicStaticPairs; // Enable physx::PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS
		bool _enableAveragePoint; // Enable physx::PxSceneFlag::eENABLE_AVERAGE_POINT
		bool _enableEnhancedDeterminism; // Enable physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM
		bool _enableCCD; // Enable physx::PxSceneFlag::eENABLE_CCD
		bool _removeDamping;
	};
}
