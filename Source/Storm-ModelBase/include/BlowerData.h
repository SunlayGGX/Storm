#pragma once


namespace Storm
{
	enum class BlowerType;

	struct BlowerData
	{
	public:
		BlowerData();

	public:
		std::size_t _id;

		float _startTimeInSeconds;
		float _stopTimeInSeconds;
		float _fadeInTimeInSeconds;
		float _fadeOutTimeInSeconds;

		Storm::BlowerType _blowerType;

		Storm::Vector3 _blowerDimension;
		float _radius;

		Storm::Vector3 _blowerPosition;

		Storm::Vector3 _blowerForce;
	};
}
