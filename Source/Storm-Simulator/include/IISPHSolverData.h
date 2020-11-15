#pragma once


namespace Storm
{
	struct IISPHSolverData
	{
	public:
		Storm::Vector3 _nonPressureAcceleration;
		Storm::Vector3 _predictedAcceleration; // Non pressure accel + pressure accel

		Storm::Vector3 _predictedVelocity;

		float _advectedDensity;
		Storm::Vector3 _dii;
		float _aii;
	};
}
