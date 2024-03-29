#pragma once


namespace Storm
{
	enum class BlowerType;

	struct SceneBlowerConfig
	{
	public:
		SceneBlowerConfig();

	public:
		unsigned int _blowerId;

		float _startTimeInSeconds;
		float _stopTimeInSeconds;
		float _fadeInTimeInSeconds;
		float _fadeOutTimeInSeconds;

		Storm::BlowerType _blowerType;

		Storm::Vector3 _blowerDimension;
		float _radius;
		float _downRadius;
		float _upRadius;
		float _height;

		Storm::Vector3 _blowerPosition;

		Storm::Vector3 _blowerForce;

		float _vorticeCoeff;
		bool _applyVortice;
	};
}
