#pragma once


namespace Storm
{
	struct GeneralSimulationData
	{
	public:
		GeneralSimulationData();

	public:
		Storm::Vector3 _gravity;
		float _particleRadius;
		float _kernelCoefficient;

		float _physicsTimeInSeconds;

		bool _startPaused;
	};
}