#include "FluidParticleSystem.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "ISpacePartitionerManager.h"

#include "SceneSimulationConfig.h"
#include "SceneFluidConfig.h"

#include "PartitionSelection.h"
#include "NeighborParticleReferral.h"

#include "SemiImplicitEulerSolver.h"

#include "Kernel.h"

#include "IBlower.h"

#include "MassCoeffHandler.h"

#include "RunnerHelper.h"
#include "FastOperation.h"

#include "ParticleSystemUtils.h"

#include "UIFieldBase.h"
#include "UIFieldContainer.h"
#include "UIField.h"

#define STORM_HIJACKED_TYPE float
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE

#define STORM_WANTED_DENSITY_FIELD_NAME "Density0"


Storm::FluidParticleSystem::FluidParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions) :
	Storm::ParticleSystem{ particleSystemIndex, std::move(worldPositions) },
	_fields{ std::make_unique<Storm::UIFieldContainer>() }
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();
	const Storm::SceneFluidConfig &fluidConfig = configMgr.getSceneFluidConfig();

	_restDensity = fluidConfig._density;
	_gravityEnabled = fluidConfig._gravityEnabled;

	_wantedDensity = _restDensity;

	const std::size_t particleCount = this->getParticleCount();

	const float particleDiameter = sceneSimulationConfig._particleRadius * 2.f;

	if (std::isnan(fluidConfig._particleVolume))
	{
		const float baseV = particleDiameter * particleDiameter * particleDiameter;

		_massCoeffControlHandler = std::make_unique<Storm::MassCoeffHandler>();
		_massCoeffControlHandler->bindListenerToReducedMassCoefficientChanged([this, baseV, updateMasses = fluidConfig._massCoeffControlConfig._updateMasses](const float newMassCoeff)
		{
			_particleVolume = newMassCoeff * baseV;
			if (updateMasses)
			{
				Storm::runParallel(_masses, [this](float &currentPMass)
				{
					currentPMass = _particleVolume * _wantedDensity;
				});
			}
		});

		_particleVolume = _massCoeffControlHandler->getReducedMassCoeff() * baseV;
	}
	else
	{
		_particleVolume = fluidConfig._particleVolume;
	}

	_masses.resize(particleCount, _particleVolume * _restDensity);
	_densities.resize(particleCount);
	_pressure.resize(particleCount);
	_tmpBlowerForces.resize(particleCount, Storm::Vector3::Zero());
	
	_velocityPreTimestep.resize(particleCount);
	
	(*_fields)
		.bindField(STORM_WANTED_DENSITY_FIELD_NAME, _wantedDensity);
}

Storm::FluidParticleSystem::FluidParticleSystem(unsigned int particleSystemIndex, std::size_t particleCount) :
	Storm::ParticleSystem{ particleSystemIndex, particleCount },
	_fields{ std::make_unique<Storm::UIFieldContainer>() }
{
	_densities.resize(particleCount);
	_pressure.resize(particleCount);
	_tmpBlowerForces.resize(particleCount);

	// It will be overwritten by the record afterward but just to make it non stupid in case we have an old record that did not support the feature at the time.
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::SceneFluidConfig &fluidConfig = configMgr.getSceneFluidConfig();

	_wantedDensity = fluidConfig._density;

	(*_fields)
		.bindField(STORM_WANTED_DENSITY_FIELD_NAME, _wantedDensity);

	// No need to init the other thing since this constructor is only to be used in replay mode and we won't use them.
}

Storm::FluidParticleSystem::~FluidParticleSystem() = default;

void Storm::FluidParticleSystem::onIterationStart()
{
	Storm::ParticleSystem::onIterationStart();

#if defined(DEBUG) || defined(_DEBUG)
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	if (!configMgr.isInReplayMode())
	{
		const std::size_t particleCount = this->getParticleCount();

		assert(
			_masses.size() == particleCount &&
			_densities.size() == particleCount &&
			_velocityPreTimestep.size() == particleCount &&
			_pressure.size() == particleCount &&
			_tmpBlowerForces.size() == particleCount &&
			"Particle count mismatch detected! An array of particle property has not the same particle count than the other!"
		);
	}
#endif

	if (_massCoeffControlHandler)
	{
		_massCoeffControlHandler->update();
	}
}

void Storm::FluidParticleSystem::onSubIterationStart(const Storm::ParticleSystemContainer &allParticleSystems, const std::vector<std::unique_ptr<Storm::IBlower>> &blowers)
{
	Storm::ParticleSystem::onSubIterationStart(allParticleSystems, blowers);

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

	const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();

	const Storm::Vector3 gravityAccel = _gravityEnabled ? sceneSimulationConfig._gravity : Storm::Vector3::Zero();

	Storm::runParallel(_force, [this, &gravityAccel, &blowers](Storm::Vector3 &currentPForce, const std::size_t currentPIndex)
	{
		this->internalInitializeForce(gravityAccel, blowers, currentPForce, currentPIndex);

		_velocityPreTimestep[currentPIndex] = _velocity[currentPIndex];
	});
}

void Storm::FluidParticleSystem::onIterationEnd()
{
	Storm::ParticleSystem::onIterationEnd();

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::SceneFluidConfig &fluidConfig = configMgr.getSceneFluidConfig();
	if (fluidConfig._smoothingRestDensity)
	{
		float avgDensity = std::reduce(std::execution::par, std::begin(_densities), std::end(_densities), 0.f);
		avgDensity /= static_cast<float>(this->getParticleCount());

		// This comes from Xavier Chermain implementation for its mémoire.
		float tmp = _restDensity * 0.001f;
		if (_wantedDensity > avgDensity)
		{
			tmp = _wantedDensity - tmp;
		}
		else
		{
			tmp = _wantedDensity + tmp;
		}

		this->setParticleSystemWantedDensity(tmp);
	}
}

bool Storm::FluidParticleSystem::isFluids() const noexcept
{
	return true;
}

bool Storm::FluidParticleSystem::isStatic() const noexcept
{
	return false;
}

bool Storm::FluidParticleSystem::isWall() const noexcept
{
	return false;
}

void Storm::FluidParticleSystem::setPositions(std::vector<Storm::Vector3> &&positions)
{
	_positions = std::move(positions);
	_isDirty = true;
}

void Storm::FluidParticleSystem::setVelocity(std::vector<Storm::Vector3> &&velocities)
{
	if (_velocityPreTimestep.size() == velocities.size()) STORM_LIKELY
	{
		if (Storm::FastOperation::canUseAVX512())
		{
			Storm::FastOperation::copyMemory_V2<Storm::SIMDUsageMode::AVX512>(velocities, _velocityPreTimestep);
		}
		else if (Storm::FastOperation::canUseSIMD())
		{
			Storm::FastOperation::copyMemory_V2<Storm::SIMDUsageMode::SSE>(velocities, _velocityPreTimestep);
		}
		else
		{
			Storm::FastOperation::copyMemory_V2<Storm::SIMDUsageMode::SISD>(velocities, _velocityPreTimestep);
		}
	}
	else
	{
		_velocityPreTimestep = velocities;
	}
	_velocity = std::move(velocities);
}

void Storm::FluidParticleSystem::setForces(std::vector<Storm::Vector3> &&forces)
{
	_force = std::move(forces);
}

void Storm::FluidParticleSystem::setDensities(std::vector<float> &&densities)
{
	_densities = std::move(densities);
}

void Storm::FluidParticleSystem::setPressures(std::vector<float> &&pressures)
{
	_pressure = std::move(pressures);
}

void Storm::FluidParticleSystem::setVolumes(std::vector<float> &&/*volumes*/)
{

}

void Storm::FluidParticleSystem::setMasses(std::vector<float> &&masses)
{
	_masses = std::move(masses);
}

void Storm::FluidParticleSystem::setNormals(std::vector<Storm::Vector3> &&/*normals*/)
{
	
}

void Storm::FluidParticleSystem::setTmpPressureForces(std::vector<Storm::Vector3> &&tmpPressureForces)
{
	_tmpPressureForce = std::move(tmpPressureForces);
}

void Storm::FluidParticleSystem::setTmpViscosityForces(std::vector<Storm::Vector3> &&tmpViscoForces)
{
	_tmpViscosityForce = std::move(tmpViscoForces);
}

void Storm::FluidParticleSystem::setTmpDragForces(std::vector<Storm::Vector3> &&tmpDragForces)
{
	_tmpDragForce = std::move(tmpDragForces);
}

void Storm::FluidParticleSystem::setTmpBernoulliDynamicPressureForces(std::vector<Storm::Vector3>&& tmpDynamicQForces)
{
	_tmpBernoulliDynamicPressureForce = std::move(tmpDynamicQForces);
}

void Storm::FluidParticleSystem::setTmpNoStickForces(std::vector<Storm::Vector3> &&tmpNoStickForces)
{
	_tmpNoStickForce = std::move(tmpNoStickForces);
}

void Storm::FluidParticleSystem::setTmpCoendaForces(std::vector<Storm::Vector3> &&tmpCoendaForces)
{
	_tmpCoendaForce = std::move(tmpCoendaForces);
}

void Storm::FluidParticleSystem::setTmpPressureDensityIntermediaryForces(std::vector<Storm::Vector3> &&tmpPressuresIntermediaryForces)
{
	_tmpPressureDensityIntermediaryForce = std::move(tmpPressuresIntermediaryForces);
}

void Storm::FluidParticleSystem::setTmpPressureVelocityIntermediaryForces(std::vector<Storm::Vector3> &&tmpPressuresIntermediaryForces)
{
	_tmpPressureVelocityIntermediaryForce = std::move(tmpPressuresIntermediaryForces);
}

void Storm::FluidParticleSystem::setTmpBlowerForces(std::vector<Storm::Vector3> &&tmpBlowerForces)
{
	_tmpBlowerForces = std::move(tmpBlowerForces);
}

void Storm::FluidParticleSystem::setParticleSystemPosition(const Storm::Vector3 &/*pSystemPosition*/)
{

}

void Storm::FluidParticleSystem::setParticleSystemTotalForce(const Storm::Vector3 &/*pSystemTotalForce*/)
{
	// Both are the same. The total force and the non physX one since physX does not handle the fluid.
	// But to prevent any mistakes when reading recordings, we'll disable this method
#if false
	this->setParticleSystemTotalForceNonPhysX(pSystemTotalForce);
#endif
}

void Storm::FluidParticleSystem::setParticleSystemWantedDensity(const float value)
{
	// If it is NaN, then this is not good value.
	if (!isnan(value))
	{
		Storm::updateField(*_fields, STORM_WANTED_DENSITY_FIELD_NAME, _wantedDensity, value);
	}
}

void Storm::FluidParticleSystem::prepareSaving(const bool replayMode)
{
	Storm::ParticleSystem::prepareSaving(replayMode);

	if (replayMode)
	{
		const std::size_t particleCount = _positions.size();
		if (_masses.size() != particleCount)
		{
			_masses.reserve(particleCount);
			Storm::setNumUninitialized_hijack(_masses, Storm::VectorHijacker{ particleCount });
		}

		const std::vector<float> &densities = this->getDensities();

		Storm::runParallel(_masses, [&densities, pVolume = this->getParticleVolume()](float &currentPMass, const std::size_t currentPIndex)
		{
			currentPMass = densities[currentPIndex] * pVolume;
		});
	}
}

float Storm::FluidParticleSystem::getRestDensity() const noexcept
{
	// Wanted density is equal to the rest density if we did not enable the density smooth change.
	return _wantedDensity;
}

float Storm::FluidParticleSystem::getParticleVolume() const noexcept
{
	return _particleVolume;
}

std::vector<float>& Storm::FluidParticleSystem::getMasses() noexcept
{
	return _masses;
}

const std::vector<float>& Storm::FluidParticleSystem::getMasses() const noexcept
{
	return _masses;
}

std::vector<float>& Storm::FluidParticleSystem::getDensities() noexcept
{
	return _densities;
}

const std::vector<float>& Storm::FluidParticleSystem::getDensities() const noexcept
{
	return _densities;
}

std::vector<float>& Storm::FluidParticleSystem::getPressures() noexcept
{
	return _pressure;
}

const std::vector<float>& Storm::FluidParticleSystem::getPressures() const noexcept
{
	return _pressure;
}

std::vector<Storm::Vector3>& Storm::FluidParticleSystem::getVelocityPreTimestep() noexcept
{
	return _velocityPreTimestep;
}

const std::vector<Storm::Vector3>& Storm::FluidParticleSystem::getVelocityPreTimestep() const noexcept
{
	return _velocityPreTimestep;
}

std::vector<Storm::Vector3>& Storm::FluidParticleSystem::getTmpBlowerForces() noexcept
{
	return _tmpBlowerForces;
}

const std::vector<Storm::Vector3>& Storm::FluidParticleSystem::getTmpBlowerForces() const noexcept
{
	return _tmpBlowerForces;
}

void Storm::FluidParticleSystem::setGravityEnabled(bool enabled) noexcept
{
	_gravityEnabled = enabled;
}

// TODO : Factorize with buildSpecificParticleNeighborhoodOnParticleSystemUsingSpacePartition without performance loss (because this code is performance critical)
void Storm::FluidParticleSystem::buildNeighborhoodOnParticleSystemUsingSpacePartition(const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLength)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::ISpacePartitionerManager &spacePartitionerMgr = singletonHolder.getSingleton<Storm::ISpacePartitionerManager>();

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();

	const Storm::RawKernelMethodDelegate rawKernel = Storm::retrieveRawKernelMethod(sceneSimulationConfig._kernelMode);
	const Storm::GradKernelMethodDelegate gradKernel = Storm::retrieveGradKernelMethod(sceneSimulationConfig._kernelMode);

	Storm::runParallel(_neighborhood, [this, &allParticleSystems, &spacePartitionerMgr, &rawKernel, &gradKernel, kernelLength, kernelLengthSquared = kernelLength * kernelLength, currentSystemId = this->getId(), domainDimension = spacePartitionerMgr.getDomainDimension(), infiniteDomain = spacePartitionerMgr.isInfiniteDomainMode()](ParticleNeighborhoodArray &currentPNeighborhood, const std::size_t particleIndex)
	{
		const Storm::Vector3 &currentPPosition = _positions[particleIndex];
		if (spacePartitionerMgr.isOutsideSpaceDomain(currentPPosition)) STORM_UNLIKELY
		{
			return;
		}

		const std::vector<Storm::NeighborParticleReferral>* bundleContainingPtr;
		const std::vector<Storm::NeighborParticleReferral>* outLinkedNeighborBundle[Storm::k_neighborLinkedBunkCount];

		Storm::NeighborSearchInParam<ParticleNeighborhoodArray, decltype(rawKernel), decltype(gradKernel), Storm::k_neighborLinkedBunkCount> inParam{
			this,
			allParticleSystems,
			kernelLength,
			kernelLengthSquared,
			currentSystemId,
			currentPNeighborhood,
			particleIndex,
			currentPPosition,
			bundleContainingPtr,
			outLinkedNeighborBundle,
			rawKernel,
			gradKernel,
			domainDimension,
			true
		};

		if (infiniteDomain)
		{
			// Get all particles referrals that are near the current particle position. First, get all particles inside the fluid partitioned space...
			spacePartitionerMgr.getAllBundlesInfinite(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::Fluid, inParam._reflectedModality);

			if (inParam._reflectedModality->_summary != Storm::OutReflectedModalityEnum::None)
			{
				Storm::searchForNeighborhood<true, true>(inParam);

				inParam._isFluid = false;

				spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::StaticRigidBody);
				Storm::searchForNeighborhood<false, true>(inParam);

				spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::DynamicRigidBody);
				Storm::searchForNeighborhood<false, true>(inParam);
			}
			else
			{
				Storm::searchForNeighborhood<true, false>(inParam);

				inParam._isFluid = false;

				spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::StaticRigidBody);
				Storm::searchForNeighborhood<false, false>(inParam);

				spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::DynamicRigidBody);
				Storm::searchForNeighborhood<false, false>(inParam);
			}
		}
		else
		{
			// Get all particles referrals that are near the current particle position. First, get all particles inside the fluid partitioned space...
			spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::Fluid);
			Storm::searchForNeighborhood<true, false>(inParam);

			inParam._isFluid = false;

			spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::StaticRigidBody);
			Storm::searchForNeighborhood<false, false>(inParam);

			spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::DynamicRigidBody);
			Storm::searchForNeighborhood<false, false>(inParam);
		}
	});
}

// TODO : Factorize with buildNeighborhoodOnParticleSystemUsingSpacePartition without performance loss (because this code is performance critical)
void Storm::FluidParticleSystem::buildSpecificParticleNeighborhoodOnParticleSystemUsingSpacePartition(const Storm::ParticleSystemContainer &allParticleSystems, const std::size_t particleIndex, const float kernelLength)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::ISpacePartitionerManager &spacePartitionerMgr = singletonHolder.getSingleton<Storm::ISpacePartitionerManager>();

	const Storm::Vector3 &currentPPosition = _positions[particleIndex];
	if (spacePartitionerMgr.isOutsideSpaceDomain(currentPPosition)) STORM_UNLIKELY
	{
		return;
	}

	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();

	const std::vector<Storm::NeighborParticleReferral>* bundleContainingPtr;
	const std::vector<Storm::NeighborParticleReferral>* outLinkedNeighborBundle[Storm::k_neighborLinkedBunkCount];

	Storm::NeighborSearchInParam<ParticleNeighborhoodArray, decltype(Storm::retrieveRawKernelMethod(sceneSimulationConfig._kernelMode)), decltype(Storm::retrieveGradKernelMethod(sceneSimulationConfig._kernelMode)), Storm::k_neighborLinkedBunkCount> inParam{
		this,
		allParticleSystems,
		kernelLength,
		kernelLength * kernelLength,
		this->getId(),
		_neighborhood[particleIndex],
		particleIndex,
		currentPPosition,
		bundleContainingPtr,
		outLinkedNeighborBundle,
		Storm::retrieveRawKernelMethod(sceneSimulationConfig._kernelMode),
		Storm::retrieveGradKernelMethod(sceneSimulationConfig._kernelMode),
		spacePartitionerMgr.getDomainDimension(),
		true
	};

	if (spacePartitionerMgr.isInfiniteDomainMode())
	{
		// Get all particles referrals that are near the current particle position. First, get all particles inside the fluid partitioned space...
		spacePartitionerMgr.getAllBundlesInfinite(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::Fluid, inParam._reflectedModality);

		if (inParam._reflectedModality->_summary != Storm::OutReflectedModalityEnum::None)
		{
			Storm::searchForNeighborhood<true, true>(inParam);

			inParam._isFluid = false;

			spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::StaticRigidBody);
			Storm::searchForNeighborhood<false, true>(inParam);

			spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::DynamicRigidBody);
			Storm::searchForNeighborhood<false, true>(inParam);
		}
		else
		{
			Storm::searchForNeighborhood<true, false>(inParam);

			inParam._isFluid = false;

			spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::StaticRigidBody);
			Storm::searchForNeighborhood<false, false>(inParam);

			spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::DynamicRigidBody);
			Storm::searchForNeighborhood<false, false>(inParam);
		}
	}
	else
	{
		// Get all particles referrals that are near the current particle position. First, get all particles inside the fluid partitioned space...
		spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::Fluid);
		Storm::searchForNeighborhood<true, false>(inParam);

		inParam._isFluid = false;

		spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::StaticRigidBody);
		Storm::searchForNeighborhood<false, false>(inParam);

		spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::DynamicRigidBody);
		Storm::searchForNeighborhood<false, false>(inParam);
	}
}

bool Storm::FluidParticleSystem::computeVelocityChange(float deltaTimeInSec, float highVelocityThresholdSquared)
{
	std::atomic<bool> highSpeed = false;

	Storm::runParallel(_force, [this, deltaTimeInSec, &highSpeed, highVelocityThresholdSquared](const Storm::Vector3 &currentForce, const std::size_t currentPIndex)
	{
		Storm::Vector3 &currentPVelocity = _velocity[currentPIndex];

		Storm::SemiImplicitEulerVelocitySolver solver{ _masses[currentPIndex], currentForce, deltaTimeInSec };

		currentPVelocity += solver._velocityVariation;
		if (currentPVelocity.squaredNorm() > highVelocityThresholdSquared)
		{
			highSpeed = true;
		}
	});

	return highSpeed;
}

void Storm::FluidParticleSystem::updatePosition(float deltaTimeInSec, bool)
{
	Storm::runParallel(_velocity, [this, deltaTimeInSec](Storm::Vector3 &currentPVelocity, const std::size_t currentPIndex)
	{
		Storm::SemiImplicitEulerPositionSolver solver{ currentPVelocity, deltaTimeInSec };

		_positions[currentPIndex] += solver._positionDisplacment;

		if (!_isDirty)
		{
			// Displacement under 0.001mm won't be considered... 
			constexpr const float k_epsilon = 0.000001f;

			if (fabs(solver._positionDisplacment.x()) > k_epsilon ||
				fabs(solver._positionDisplacment.y()) > k_epsilon ||
				fabs(solver._positionDisplacment.z()) > k_epsilon)
			{
				_isDirty = true;
			}
		}
	});
}

void Storm::FluidParticleSystem::revertToCurrentTimestep(const std::vector<std::unique_ptr<Storm::IBlower>> &blowers)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::SceneSimulationConfig &sceneSimulationConfig = configMgr.getSceneSimulationConfig();

	const Storm::Vector3 gravityAccel = _gravityEnabled ? sceneSimulationConfig._gravity : Storm::Vector3::Zero();

	Storm::runParallel(_force, [this, &blowers, &gravityAccel](Storm::Vector3 &force, const std::size_t currentPIndex)
	{
		this->internalInitializeForce(gravityAccel, blowers, force, currentPIndex);
		_velocity[currentPIndex] = _velocityPreTimestep[currentPIndex];
	});
}

void Storm::FluidParticleSystem::internalInitializeForce(const Storm::Vector3 &gravityAccel, const std::vector<std::unique_ptr<Storm::IBlower>> &blowers, Storm::Vector3 &force, const std::size_t currentPIndex)
{
	const float currentPMass = _masses[currentPIndex];
	force = currentPMass * gravityAccel;

	Storm::Vector3 &currentPBlowerForce = _tmpBlowerForces[currentPIndex];
	currentPBlowerForce.setZero();

	const Storm::Vector3 &currentPPosition = _positions[currentPIndex];
	for (const std::unique_ptr<Storm::IBlower> &blowerUPtr : blowers)
	{
		blowerUPtr->applyForce(currentPPosition, currentPBlowerForce);
	}

	force += currentPBlowerForce;

	this->resetParticleTemporaryForces(currentPIndex);
}
