#pragma once


namespace Storm
{
	enum class CollisionType;

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

		bool _static;
	};
}
