#pragma once


namespace Storm
{
	struct PCISPHSolverData
	{
	public:
		Storm::Vector3 _nonPressureAcceleration;
		Storm::Vector3 _predictedAcceleration; // Non pressure accel + pressure accel

		Storm::Vector3 _srcVelocity;
		Storm::Vector3 _currentVelocity;

		Storm::Vector3 _srcPosition;
		Storm::Vector3 _currentPosition;

		float _predictedDensity;
		float _predictedPressure;
	};
}
