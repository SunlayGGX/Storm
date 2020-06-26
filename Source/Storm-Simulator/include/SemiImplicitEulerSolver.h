#pragma once


namespace Storm
{
	class SemiImplicitEulerSolver
	{
	public:
		SemiImplicitEulerSolver(float mass, const Storm::Vector3 &totalForce, const Storm::Vector3 &currentVelocity, float deltaTimeInSec);

	public:
		Storm::Vector3 _velocityVariation;
		Storm::Vector3 _positionDisplacment;
	};
}
