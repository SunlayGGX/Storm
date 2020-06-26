#include "SemiImplicitEulerSolver.h"



Storm::SemiImplicitEulerSolver::SemiImplicitEulerSolver(float mass, const Storm::Vector3 &totalForce, const Storm::Vector3 &currentVelocity, float deltaTimeInSec) :
	_velocityVariation{ totalForce * (deltaTimeInSec / mass) }
{
	_positionDisplacment = deltaTimeInSec * (currentVelocity + _velocityVariation);
}
