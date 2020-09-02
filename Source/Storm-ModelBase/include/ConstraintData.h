#pragma once


namespace Storm
{
	struct ConstraintData
	{
	public:
		ConstraintData();

	public:
		unsigned int _rigidBodyId1;
		unsigned int _rigidBodyId2;
		float _constraintsLength;
	};
}
