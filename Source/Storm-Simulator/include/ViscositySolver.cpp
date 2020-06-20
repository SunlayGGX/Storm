#include "ViscositySolver.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "SimulatorManager.h"

#include "ParticleSystem.h"

#include "FluidData.h"

#include "SpikyKernel.h"


namespace
{
	class ViscosityComputator
	{
	public:
		ViscosityComputator(float currentParticleMass, float kernelLength, float currentParticleDensity, const Storm::Vector3 &currentParticleVelocity) :
			_kernelLength{ kernelLength },
			_currentParticleDensity{ currentParticleDensity },
			_currentParticleVelocity{ currentParticleVelocity },
			_artificialViscoEpsilonCoeff{ 0.01f * kernelLength * kernelLength }
		{
			const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
			const Storm::FluidData &fluidConfData = configMgr.getFluidData();

			_precomputedCoeff = currentParticleMass * 2.f * kernelLength * fluidConfData._dynamicViscosity * fluidConfData._soundSpeed;
		}

		Storm::Vector3 operator()(float neighborMass, float neighborDensity, const Storm::Vector3 &neighborVelocity, const Storm::NeighborParticleInfo &neighborParticleInfo) const
		{
			const Storm::Vector3 vij = _currentParticleVelocity - neighborVelocity;

			const float coeff = neighborMass * _precomputedCoeff / ((_currentParticleDensity + neighborDensity) * (neighborParticleInfo._vectToParticleSquaredNorm + _artificialViscoEpsilonCoeff));

			const float completeArtificialViscosityCoefficient = coeff * std::min(vij.dot(neighborParticleInfo._positionDifferenceVector), 0.f);

			return completeArtificialViscosityCoefficient * vij.normalized();
		}

	private:
		const float _kernelLength;
		float _precomputedCoeff;
		const float _artificialViscoEpsilonCoeff;
		const float _currentParticleDensity;
		const Storm::Vector3 &_currentParticleVelocity;
	};
}

Storm::Vector3 Storm::ViscositySolver::computeViscosityForcePCISPH(float currentParticleMass, float currentParticleDensity, const Storm::Vector3 &currentParticleVelocity, const std::vector<Storm::NeighborParticleInfo> &particleNeighborhood)
{
	Storm::Vector3 result{ 0.f, 0.f, 0.f };

	const Storm::SimulatorManager &simulMgr = Storm::SimulatorManager::instance();
	const float currentKernelLength = simulMgr.getKernelLength();

	ViscosityComputator viscosityComputator{ currentParticleMass, currentKernelLength, currentParticleDensity, currentParticleVelocity };
	Storm::GradientSpikyKernel gradSpikyKernel{ currentKernelLength };

	for (const Storm::NeighborParticleInfo &neighboorIdent : particleNeighborhood)
	{
		const Storm::ParticleSystem &neighborhoodContainingPSystem = *neighboorIdent._containingParticleSystem;

		const float massPerParticle = neighborhoodContainingPSystem.getMassPerParticle();
		const float neighborDensity = neighborhoodContainingPSystem.getDensities()[neighboorIdent._particleIndex];
		const Storm::Vector3 &neighborVelocity = neighborhoodContainingPSystem.getVelocity()[neighboorIdent._particleIndex];

		result += viscosityComputator(massPerParticle, neighborDensity, neighborVelocity, neighboorIdent) * gradSpikyKernel(neighboorIdent._vectToParticleNorm);
	}

	return result;
}
