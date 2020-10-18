#include "PCISPHSolver.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "ParticleSystem.h"

#include "Kernel.h"


Storm::PCISPHSolver::PCISPHSolver(const float k_kernelLength)
{
	Storm::Vector3d uniformNeighborStiffnessCoeffSum = Storm::Vector3d::Zero();
	double uniformNeighborStiffnessCoeffProd = 0.0;

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();

	const float coordinateBegin = -k_kernelLength;
	const float particleDiameter = generalSimulData._particleRadius * 2.f;
	const unsigned int uniformComputationEndIter = std::max(static_cast<unsigned int>(std::ceilf(k_kernelLength / generalSimulData._particleRadius)) - 1, static_cast<unsigned int>(0));

	const auto k_gradMethod = Storm::retrieveGradKernelMethod(generalSimulData._kernelMode);

	Storm::Vector3 neighborUniformPos;

	const double k_templatePVolume = Storm::ParticleSystem::computeParticleDefaultVolume();
	const double k_templatePVolumeSquared = k_templatePVolume * k_templatePVolume;

	for (unsigned int xIter = 1; xIter < uniformComputationEndIter; ++xIter)
	{
		neighborUniformPos.x() = coordinateBegin + static_cast<float>(xIter) * particleDiameter;
		for (unsigned int yIter = 1; yIter < uniformComputationEndIter; ++yIter)
		{
			neighborUniformPos.y() = coordinateBegin + static_cast<float>(yIter) * particleDiameter;
			for (unsigned int zIter = 1; zIter < uniformComputationEndIter; ++zIter)
			{
				neighborUniformPos.z() = coordinateBegin + static_cast<float>(zIter) * particleDiameter;

				const float norm = neighborUniformPos.norm();
				if (norm < k_kernelLength && norm > 0.0000001f)
				{
					const Storm::Vector3 gradVal = k_gradMethod(k_kernelLength, neighborUniformPos, norm);

					// The values goes quickly really high, therefore k_templatePVolumeSquared is to reduce them to prevent going into the inaccurate values of the data type.
					uniformNeighborStiffnessCoeffSum += gradVal.cast<double>() * k_templatePVolumeSquared;
					uniformNeighborStiffnessCoeffProd += gradVal.squaredNorm() * k_templatePVolumeSquared;
				}
			}
		}
	}

	_kUniformStiffnessConstCoefficient = static_cast<float>(-0.5 / (uniformNeighborStiffnessCoeffSum.squaredNorm() + uniformNeighborStiffnessCoeffProd));
}

void Storm::PCISPHSolver::execute(const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &particleSystems, const float k_kernelLength, const float k_deltaTime)
{
	STORM_NOT_IMPLEMENTED;

	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::GeneralSimulationData &generalSimulData = configMgr.getGeneralSimulationData();
	const Storm::FluidData &fluidConfigData = configMgr.getFluidData();

	const float k_maxDensityError = generalSimulData._maxDensityError;
	const unsigned int k_maxPredictionIter = generalSimulData._maxPredictIteration;
	unsigned int currentPredictionIter = 0;

	// First : compute stiffness constant coeff kPCI.

	const float k_templatePStiffnessCoeffK = _kUniformStiffnessConstCoefficient / (k_deltaTime * k_deltaTime);
}