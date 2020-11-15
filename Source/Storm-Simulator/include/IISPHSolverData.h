#pragma once


namespace Storm
{
	struct IISPHSolverData
	{
	public:
		Storm::Vector3 _nonPressureAcceleration;
		Storm::Vector3 _predictedAcceleration; // Non pressure accel + pressure accel

		Storm::Vector3 _predictedVelocity;

		float _predictedPressure;

		float _advectedDensity;
		float _aii;
		Storm::Vector3 _dii;
		Storm::Vector3 _diiP;
		Storm::Vector3 _dijPj;
	};
}
