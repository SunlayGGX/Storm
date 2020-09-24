#include "SemiImplicitEulerSolver.h"



Storm::SemiImplicitEulerVelocitySolver::SemiImplicitEulerVelocitySolver(float mass, const Storm::Vector3 &totalForce, const Storm::Vector3 &currentVelocity, float deltaTimeInSec) :
	_velocityVariation{ totalForce * (deltaTimeInSec / mass) }
{

}

Storm::SemiImplicitEulerPositionSolver::SemiImplicitEulerPositionSolver(const Storm::Vector3 &currentVelocity, float deltaTimeInSec) :
	_positionDisplacment{ deltaTimeInSec * currentVelocity }
{

}
