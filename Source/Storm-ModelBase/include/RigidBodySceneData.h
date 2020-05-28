#pragma once


namespace Storm
{
	struct RigidBodySceneData
	{
	public:
		unsigned int _rigidBodyID;
		std::string _meshFilePath;

		Storm::Vector3 _translation;
		Storm::Vector3 _rotation;
		Storm::Vector3 _scale;

		bool _static;
	};
}
