#pragma once


namespace Storm
{
	struct ConstraintData
	{
	public:
		ConstraintData();

	public:
		std::size_t _constraintId;

		unsigned int _rigidBodyId1;
		unsigned int _rigidBodyId2;
		float _constraintsLength;
		Storm::Vector3 _rigidBody1LinkTranslationOffset;
		Storm::Vector3 _rigidBody2LinkTranslationOffset;

		bool _preventRotations;
		bool _shouldVisualize;
	};
}
