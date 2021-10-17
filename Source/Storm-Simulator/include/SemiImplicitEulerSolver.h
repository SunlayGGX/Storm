#pragma once


namespace Storm
{
	class SemiImplicitEulerVelocitySolver
	{
	public:
		SemiImplicitEulerVelocitySolver(float mass, const Storm::Vector3 &totalForce, float deltaTimeInSec);

	public:
		Storm::Vector3 _velocityVariation;
	};

	class SemiImplicitEulerPositionSolver
	{
	public:
		SemiImplicitEulerPositionSolver(const Storm::Vector3 &currentVelocity, float deltaTimeInSec);

	public:
		Storm::Vector3 _positionDisplacment;
	};
}
