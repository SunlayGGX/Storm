#include "DensitySolver.h"

#include "SingletonHolder.h"
#include "SimulatorManager.h"


namespace
{
	class Poly6Kernel
	{
	public:
		Poly6Kernel(float kernelLength) :
			_kernelLengthSquared{ kernelLength * kernelLength }
		{
			constexpr const float constexprCoeff = static_cast<float>(315.0 / (64.0 * M_PI));

			// h^4
			const float kernelFourth = _kernelLengthSquared * _kernelLengthSquared;

			// (315 / 64 * pi * h^9)
			_precomputedCoeff = constexprCoeff / (kernelFourth * kernelFourth * kernelLength);
		}

		float operator()(float particleDistSquared) const
		{
			return _precomputedCoeff * (_kernelLengthSquared - particleDistSquared);
		}

	private:
		float _kernelLengthSquared;
		float _precomputedCoeff;
	};
}


float Storm::DensitySolver::computeDensityPCISPH(float particleMass, const std::vector<Storm::ParticleIdentifier> &particleNeighborhood)
{
	float resultDensity = 0.f;

	const Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();

	Poly6Kernel poly6Kernel{ simulMgr.getKernelLength() };

	for (const Storm::ParticleIdentifier &neighborhoodParticleIdent : particleNeighborhood)
	{
		resultDensity += particleMass * poly6Kernel(neighborhoodParticleIdent._vectToParticleSquaredNorm);

		// TODO : Compute corrected density using Shepard filter (Xavier Chermain Memoire : equation 2.8) to handle boundary handling.
	}

	return resultDensity;
}
