#pragma once


namespace Storm
{
	struct DFSPHSolverData
	{
	public:
		Storm::Vector3 _nonPressureAcceleration;

		float _kCoeff;

		float _densityAdv;
		Storm::Vector3 _predictedVelocity;
	};
}
