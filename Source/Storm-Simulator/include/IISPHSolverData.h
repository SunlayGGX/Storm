#pragma once


namespace Storm
{
	struct IISPHSolverData
	{
	public:
		Storm::Vector3 _nonPressureAcceleration;
		Storm::Vector3 _predictedAcceleration; // Non pressure accel + pressure accel

	};
}
