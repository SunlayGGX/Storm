#include "PCISPHSolver.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"
#include "FluidData.h"

#include "ParticleSystem.h"


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
	const float k_templatePVolume = Storm::ParticleSystem::computeParticleDefaultVolume();

	const float k_templatePStiffnessCoeffK = -0.5f / (k_templatePVolume * k_templatePVolume * k_deltaTime * k_deltaTime);
}