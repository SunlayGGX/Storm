#pragma once


namespace Storm
{
	enum class BlowerType;

	struct BlowerData
	{
	public:
		BlowerData();

	public:
		unsigned int _blowerId;

		float _startTimeInSeconds;
		float _stopTimeInSeconds;
		float _fadeInTimeInSeconds;
		float _fadeOutTimeInSeconds;

		Storm::BlowerType _blowerType;

		Storm::Vector3 _blowerDimension;
		float _radius;
		float _height;

		Storm::Vector3 _blowerPosition;

		Storm::Vector3 _blowerForce;

		bool _makeRigidBody;
	};
}
