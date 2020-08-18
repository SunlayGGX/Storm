#pragma once


namespace Storm
{
	enum class BlowerType;

	struct BlowerData
	{
	public:
		BlowerData();

	public:
		float _startTimeInSeconds;
		float _stopTimeInSeconds;
		float _fadeInTimeInSeconds;
		float _fadeOutTimeInSeconds;

		Storm::BlowerType _blowerType;

		Storm::Vector3 _blowerDimension;
		Storm::Vector3 _blowerPosition;

		Storm::Vector3 _blowerForce;
	};
}
