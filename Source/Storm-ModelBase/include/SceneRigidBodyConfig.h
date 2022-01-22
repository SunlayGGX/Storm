#pragma once


namespace Storm
{
	enum class CollisionType;
	enum class GeometryType;
	enum class InsideParticleRemovalTechnique;
	enum class LayeringGenerationTechnique;
	enum class VolumeComputationTechnique;

	struct GeometryConfig;

	struct SceneRigidBodyConfig
	{
	public:
		SceneRigidBodyConfig();

	public:
		unsigned int _rigidBodyID;
		std::string _meshFilePath;

		Storm::Vector3 _translation;
		Storm::Rotation _rotation;
		Storm::Vector3 _scale;

		Storm::CollisionType _collisionShape;

		Storm::Vector4 _color;
		bool _separateColoring;

		float _mass;

		float _viscosity;
		float _noStickCoeff;
		float _dragCoefficient;
		float _coandaCoefficient;

		float _reducedVolumeCoeff;

		bool _static;
		bool _isWall;

		// Actually, I don't really know what those coefficient are physically speaking (units, formulas, ...), but PhysX seems to ask for them to create a rigidbody material.
		// http://docs.garagegames.com/torque-3d/reference/classPxMaterial.html
		float _staticFrictionCoefficient;
		float _dynamicFrictionCoefficient;
		float _restitutionCoefficient;
		float _angularVelocityDamping;

		bool _isTranslationFixed;

		Storm::InsideParticleRemovalTechnique _insideRbFluidDetectionMethodEnum;

		unsigned int _layerCount;
		Storm::LayeringGenerationTechnique _layerGenerationMode;
		std::shared_ptr<const Storm::GeometryConfig> _geometry;

		bool _enforceNormalsCoherency;

		bool _fixedSimulationVolume;
		Storm::VolumeComputationTechnique _volumeComputationTechnique;

		std::string _animationXmlPath;
		std::string _animationName;
		std::string _animationXmlContent; // This is not the responsibility of the config modules to parse the animation.
	};
}
