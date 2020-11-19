#pragma once


namespace Storm
{
	struct DFSPHSolverData
	{
	public:
		Storm::Vector3 _nonPressureAcceleration;
		Storm::Vector3 _predictedAcceleration; // Non pressure accel + pressure accel

		float _kCoeff;

		Storm::Vector3 _predictedVelocity;
	};
}
