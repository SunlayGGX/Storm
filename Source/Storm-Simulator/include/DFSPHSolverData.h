#pragma once


namespace Storm
{
	struct DFSPHSolverData
	{
	public:
		Storm::Vector3 _nonPressureAcceleration;

		float _kCoeff;

		float _predictedDensity;
		Storm::Vector3 _predictedVelocity;
	};
}
