#pragma once


namespace Storm
{
	struct SceneConstraintConfig
	{
	public:
		SceneConstraintConfig();

	public:
		unsigned int _constraintId;

		unsigned int _rigidBodyId1;
		unsigned int _rigidBodyId2;
		float _constraintsLength;
		Storm::Vector3 _rigidBody1LinkTranslationOffset;
		Storm::Vector3 _rigidBody2LinkTranslationOffset;

		bool _preventRotations;
		bool _shouldVisualize;
	};
}