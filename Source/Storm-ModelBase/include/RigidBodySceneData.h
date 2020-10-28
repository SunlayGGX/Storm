#pragma once


namespace Storm
{
	enum class CollisionType;
	enum class InsideParticleRemovalTechnique;
	enum class LayeringGenerationTechnique;

	struct RigidBodySceneData
	{
	public:
		RigidBodySceneData();

	public:
		unsigned int _rigidBodyID;
		std::string _meshFilePath;

		Storm::Vector3 _translation;
		Storm::Vector3 _rotation;
		Storm::Vector3 _scale;

		Storm::CollisionType _collisionShape;

		float _mass;

		float _viscosity;

		bool _static;
		bool _isWall;

		// Actually, I don't really know what those coefficient are physically speaking (units, formulas, ...), but PhysX seems to ask for them to create a rigidbody material.
		// http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html
		float _staticFrictionCoefficient;
		float _dynamicFrictionCoefficient;
		float _restitutionCoefficient;

		Storm::InsideParticleRemovalTechnique _insideRbFluidDetectionMethodEnum;

		unsigned int _layerCount;
		Storm::LayeringGenerationTechnique _layerGenerationMode;
	};
}
