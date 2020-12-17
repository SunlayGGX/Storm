#include "RigidBodyParticleSystem.h"

#include "SingletonHolder.h"
#include "IPhysicsManager.h"
#include "IConfigManager.h"
#include "ISpacePartitionerManager.h"
#include "SimulatorManager.h"

#include "NeighborParticleReferral.h"
#include "PartitionSelection.h"

#include "GeneralSimulationData.h"
#include "RigidBodySceneData.h"
#include "CollisionType.h"

#include "RunnerHelper.h"
#include "Kernel.h"

#include "ParticleSystemUtils.h"

#include "MacroConfig.h"

namespace
{
	template<class NeighborhoodArray, class RawKernelMeth>
	float computeParticleDeltaVolume(const NeighborhoodArray &currentPNeighborhood, const float kernelLength, float delta, const RawKernelMeth &rawKernelMeth)
	{
		for (const Storm::NeighborParticleInfo &boundaryNeighbor : currentPNeighborhood)
		{
			delta += rawKernelMeth(kernelLength, boundaryNeighbor._vectToParticleNorm);
		}

		return delta;
	}
}


Storm::RigidBodyParticleSystem::RigidBodyParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions) :
	Storm::ParticleSystem{ particleSystemIndex, std::move(worldPositions) },
	_cachedTrackedRbRotationQuat{ Storm::Quaternion::Identity() },
	_velocityDirtyInternal{ false },
	_rbTotalForce{ Storm::Vector3::Zero() }
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::RigidBodySceneData &currentRbData = configMgr.getRigidBodyData(particleSystemIndex);

	_viscosity = currentRbData._viscosity;

	_isWall = currentRbData._isWall;
	_isStatic = _isWall || currentRbData._static;

	const std::size_t particleCount = this->getParticleCount();

	if (this->isStatic())
	{
		_staticVolumesInitValue.resize(particleCount);
	}

	_volumes.resize(particleCount);
}

Storm::RigidBodyParticleSystem::RigidBodyParticleSystem(unsigned int particleSystemIndex, const std::size_t particleCount) :
	Storm::ParticleSystem{ particleSystemIndex, particleCount },
	_cachedTrackedRbRotationQuat{ Storm::Quaternion::Identity() },
	_rbTotalForce{ Storm::Vector3::Zero() }
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::RigidBodySceneData &currentRbData = configMgr.getRigidBodyData(particleSystemIndex);
	_isWall = currentRbData._isWall;
	_isStatic = _isWall || currentRbData._static;

	_volumes.resize(particleCount);

	// No need to initialize the other arrays since we won't use them in replay mode.
}

void Storm::RigidBodyParticleSystem::initializePreSimulation(const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLength)
{
	Storm::ParticleSystem::initializePreSimulation(allParticleSystems, kernelLength);

	// This should be done only for static rigid body that doesn't move in space (optimization done by computing values that can be computed beforehand)
	if (this->isStatic())
	{
		std::vector<Storm::ParticleNeighborhoodArray> staticNeighborhood;
		staticNeighborhood.resize(_positions.size());
		for (Storm::ParticleNeighborhoodArray &particleNeighbor : staticNeighborhood)
		{
			particleNeighbor.reserve(32);
		}

		const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
		const Storm::GeneralSimulationData &generalSimulDataConfig = configMgr.getGeneralSimulationData();

		const float currentKernelZero = Storm::retrieveKernelZeroValue(generalSimulDataConfig._kernelMode);
		const Storm::RawKernelMethodDelegate rawKernelMeth = Storm::retrieveRawKernelMethod(generalSimulDataConfig._kernelMode);

		const Storm::ISpacePartitionerManager &spacePartitionerMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ISpacePartitionerManager>();
		Storm::runParallel(staticNeighborhood, [this, &allParticleSystems, kernelLength, currentKernelZero, rawKernelMeth, kernelLengthSquared = kernelLength * kernelLength, &spacePartitionerMgr, currentSystemId = this->getId()](ParticleNeighborhoodArray &currentPStaticNeighborhood, const std::size_t particleIndex)
		{
			const std::vector<Storm::NeighborParticleReferral>* bundleContainingPtr;
			const std::vector<Storm::NeighborParticleReferral>* outLinkedNeighborBundle[Storm::k_neighborLinkedBunkCount];

			const Storm::Vector3 &currentPPosition = _positions[particleIndex];

			// Get all particles referrals that are near the current particle position.
			spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::StaticRigidBody);
			Storm::searchForNeighborhood<false, true>(
				this,
				allParticleSystems,
				kernelLengthSquared,
				currentSystemId,
				currentPStaticNeighborhood,
				particleIndex,
				currentPPosition,
				*bundleContainingPtr,
				outLinkedNeighborBundle
			);

			// Initialize the static volume.
			float &currentPStaticVolumeDelta = _staticVolumesInitValue[particleIndex];
			currentPStaticVolumeDelta = computeParticleDeltaVolume(currentPStaticNeighborhood, kernelLength, currentKernelZero, rawKernelMeth);
		});
	}
}

void Storm::RigidBodyParticleSystem::onIterationStart()
{
	Storm::ParticleSystem::onIterationStart();

#if defined(_DEBUG) || defined(DEBUG)
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	if (!configMgr.isInReplayMode())
	{
		const std::size_t particleCount = this->getParticleCount();
		assert(
			_volumes.size() == particleCount &&
			"Particle count mismatch detected! An array of particle property has not the same particle count than the other!"
		);
		if (this->isStatic())
		{
			assert(
				_staticVolumesInitValue.size() == particleCount &&
				"Particle count mismatch detected! An array of particle property has not the same particle count than the other!"
			);
		}
	}
#endif
}

void Storm::RigidBodyParticleSystem::onSubIterationStart(const Storm::ParticleSystemContainer &allParticleSystems, const std::vector<std::unique_ptr<Storm::IBlower>> &blowers)
{
	Storm::ParticleSystem::onSubIterationStart(allParticleSystems, blowers);

	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();

	const Storm::GeneralSimulationData &generalSimulDataConfig = configMgr.getGeneralSimulationData();

	const float k_kernelLength = Storm::SimulatorManager::instance().getKernelLength();

	const float currentKernelZero = Storm::retrieveKernelZeroValue(generalSimulDataConfig._kernelMode);
	const Storm::RawKernelMethodDelegate rawKernelMeth = Storm::retrieveRawKernelMethod(generalSimulDataConfig._kernelMode);

	if (this->isStatic())
	{
		Storm::runParallel(_volumes, [this, currentKernelZero, rawKernelMeth, k_kernelLength](float &currentPVolume, const std::size_t currentPIndex)
		{
			// Compute the current boundary particle volume.
#if STORM_USE_OPTIMIZED_NEIGHBORHOOD_ALGORITHM
			const float initialVolumeValue = _staticVolumesInitValue[currentPIndex];
#else
			const float initialVolumeValue = currentKernelZero;
#endif
			currentPVolume = 1.f / computeParticleDeltaVolume(_neighborhood[currentPIndex], k_kernelLength, initialVolumeValue, rawKernelMeth); // ???
		});
	}
	else
	{
		Storm::runParallel(_force, [this, currentKernelZero, rawKernelMeth, k_kernelLength](Storm::Vector3 &force, const std::size_t currentPIndex)
		{
			// Initialize forces to 0
			force.setZero();
			_tmpPressureForce[currentPIndex].setZero();
			_tmpViscosityForce[currentPIndex].setZero();

			// Compute the current boundary particle volume.
			const float initialVolumeValue = currentKernelZero;

			float &currentPVolume = _volumes[currentPIndex];
			currentPVolume = 1.f / computeParticleDeltaVolume(_neighborhood[currentPIndex], k_kernelLength, initialVolumeValue, rawKernelMeth); // ???
		});
	}
}

const std::vector<float>& Storm::RigidBodyParticleSystem::getVolumes() const noexcept
{
	return _volumes;
}

float Storm::RigidBodyParticleSystem::getViscosity() const noexcept
{
	return _viscosity;
}

const Storm::Vector3& Storm::RigidBodyParticleSystem::getRbPosition() const noexcept
{
	return _cachedTrackedRbPosition;
}

const Storm::Vector3& Storm::RigidBodyParticleSystem::getRbTotalForce() const noexcept
{
	return _rbTotalForce;
}

std::vector<float>& Storm::RigidBodyParticleSystem::getVolumes() noexcept
{
	return _volumes;
}

bool Storm::RigidBodyParticleSystem::isFluids() const noexcept
{
	return false;
}

bool Storm::RigidBodyParticleSystem::isStatic() const noexcept
{
	return _isStatic;
}

bool Storm::RigidBodyParticleSystem::isWall() const noexcept
{
	return _isWall;
}

void Storm::RigidBodyParticleSystem::setPositions(std::vector<Storm::Vector3> &&positions)
{
	if (!this->isStatic())
	{
		_positions = std::move(positions);
		_isDirty = true;
	}
}

void Storm::RigidBodyParticleSystem::setVelocity(std::vector<Storm::Vector3> &&velocities)
{
	if (!this->isStatic())
	{
		_velocity = std::move(velocities);
	}
}

void Storm::RigidBodyParticleSystem::setForces(std::vector<Storm::Vector3> &&forces)
{
	if (!this->isStatic())
	{
		_force = std::move(forces);
	}
}

void Storm::RigidBodyParticleSystem::setDensities(std::vector<float> &&)
{

}

void Storm::RigidBodyParticleSystem::setPressures(std::vector<float> &&)
{

}

void Storm::RigidBodyParticleSystem::setVolumes(std::vector<float> &&volumes)
{
	_volumes = std::move(_volumes);
}

void Storm::RigidBodyParticleSystem::setMasses(std::vector<float> &&masses)
{

}

void Storm::RigidBodyParticleSystem::setTmpPressureForces(std::vector<Storm::Vector3> &&tmpPressureForces)
{
	if (!this->isStatic())
	{
		_tmpPressureForce = std::move(tmpPressureForces);
	}
}

void Storm::RigidBodyParticleSystem::setTmpViscosityForces(std::vector<Storm::Vector3> &&tmpViscoForces)
{
	if (!this->isStatic())
	{
		_tmpViscosityForce = std::move(tmpViscoForces);
	}
}

void Storm::RigidBodyParticleSystem::setParticleSystemPosition(const Storm::Vector3 &rbPosition)
{
	_cachedTrackedRbPosition = rbPosition;
}

void Storm::RigidBodyParticleSystem::setParticleSystemTotalForce(const Storm::Vector3 &rbTotalForce)
{
	_rbTotalForce = rbTotalForce;
}

void Storm::RigidBodyParticleSystem::buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared)
{
	if (!otherParticleSystem.isFluids())
	{
		Storm::runParallel(_positions, [this, kernelLengthSquared, &otherParticleSystem](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
		{
			std::vector<Storm::NeighborParticleInfo> &currentNeighborhoodToFill = _neighborhood[currentPIndex];

			const auto &otherParticleSystemPositionsArray = otherParticleSystem.getPositions();
			const std::size_t otherParticleCount = otherParticleSystemPositionsArray.size();

			for (std::size_t particleIndex = 0; particleIndex < otherParticleCount; ++particleIndex)
			{
				const Storm::Vector3 positionDifference = currentPPosition - otherParticleSystemPositionsArray[particleIndex];
				const float vectToParticleSquaredNorm = positionDifference.squaredNorm();
				if (Storm::ParticleSystem::isElligibleNeighborParticle(kernelLengthSquared, vectToParticleSquaredNorm))
				{
					currentNeighborhoodToFill.emplace_back(const_cast<Storm::ParticleSystem*>(&otherParticleSystem), particleIndex, positionDifference, vectToParticleSquaredNorm, false);
				}
			}
		});
	}
}

void Storm::RigidBodyParticleSystem::buildNeighborhoodOnParticleSystemUsingSpacePartition(const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLengthSquared)
{
	const Storm::ISpacePartitionerManager &spacePartitionerMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ISpacePartitionerManager>();

	if (this->isStatic())
	{
		Storm::runParallel(_neighborhood, [this, &allParticleSystems, kernelLengthSquared, &spacePartitionerMgr, currentSystemId = this->getId()](ParticleNeighborhoodArray &currentPNeighborhood, const std::size_t particleIndex)
		{
			const std::vector<Storm::NeighborParticleReferral>* bundleContainingPtr;
			const std::vector<Storm::NeighborParticleReferral>* outLinkedNeighborBundle[Storm::k_neighborLinkedBunkCount];

			const Storm::Vector3 &currentPPosition = _positions[particleIndex];

			// Get all particles referrals that are near the current particle position. First, rigid bodies doesn't see fluids, so do not query them...
			spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::DynamicRigidBody);
			Storm::searchForNeighborhood<false, false>(
				this,
				allParticleSystems,
				kernelLengthSquared,
				currentSystemId,
				currentPNeighborhood,
				particleIndex,
				currentPPosition,
				*bundleContainingPtr,
				outLinkedNeighborBundle
			);
		});
	}
	else
	{
		Storm::runParallel(_neighborhood, [this, &allParticleSystems, kernelLengthSquared, &spacePartitionerMgr, currentSystemId = this->getId()](ParticleNeighborhoodArray &currentPNeighborhood, const std::size_t particleIndex)
		{
			const std::vector<Storm::NeighborParticleReferral>* bundleContainingPtr;
			const std::vector<Storm::NeighborParticleReferral>* outLinkedNeighborBundle[Storm::k_neighborLinkedBunkCount];

			const Storm::Vector3 &currentPPosition = _positions[particleIndex];

			// Get all particles referrals that are near the current particle position. First, rigid bodies doesn't see fluids, so do not query them...
			spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::StaticRigidBody);
			Storm::searchForNeighborhood<false, false>(
				this,
				allParticleSystems,
				kernelLengthSquared,
				currentSystemId,
				currentPNeighborhood,
				particleIndex,
				currentPPosition,
				*bundleContainingPtr,
				outLinkedNeighborBundle
			);

			spacePartitionerMgr.getAllBundles(bundleContainingPtr, outLinkedNeighborBundle, currentPPosition, Storm::PartitionSelection::DynamicRigidBody);
			Storm::searchForNeighborhood<false, true>(
				this,
				allParticleSystems,
				kernelLengthSquared,
				currentSystemId,
				currentPNeighborhood,
				particleIndex,
				currentPPosition,
				*bundleContainingPtr,
				outLinkedNeighborBundle
			);
		});
	}
}

bool Storm::RigidBodyParticleSystem::computeVelocityChange(float, float)
{
	return false;
}

void Storm::RigidBodyParticleSystem::updatePosition(float deltaTimeInSec, bool force)
{
	if (!_isStatic || force)
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

		Storm::Vector3 currentPosition;
		Storm::Quaternion currentQuatRotation;

		const Storm::IPhysicsManager &physicsMgr = singletonHolder.getSingleton<Storm::IPhysicsManager>();

		physicsMgr.getMeshTransform(_particleSystemIndex, currentPosition, currentQuatRotation);

		if (currentPosition != _cachedTrackedRbPosition || currentQuatRotation.x() != _cachedTrackedRbRotationQuat.x() || currentQuatRotation.y() != _cachedTrackedRbRotationQuat.y() || currentQuatRotation.z() != _cachedTrackedRbRotationQuat.z() || currentQuatRotation.w() != _cachedTrackedRbRotationQuat.w())
		{
			_isDirty = true;
			_velocityDirtyInternal = true;

			const Storm::Quaternion oldRbRotationConjugateQuat = _cachedTrackedRbRotationQuat.conjugate();
			const Storm::Quaternion currentConjugateQuatRotation = currentQuatRotation.conjugate();

			Storm::runParallel(_positions, [&](Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
			{
				Storm::Vector3 &currentPVelocity = _velocity[currentPIndex];

				/* First, remove the current world space coordinate to revert to the object space coordinate (where particle were defined initially and on which PhysX was initialized). */

				// 1- Remove the translation
				Storm::Quaternion currentPosAsPureQuat{ 0.f, currentPPosition.x() - _cachedTrackedRbPosition.x(), currentPPosition.y() - _cachedTrackedRbPosition.y(), currentPPosition.z() - _cachedTrackedRbPosition.z() };

				// 2- Remove the rotation
				currentPosAsPureQuat = oldRbRotationConjugateQuat * currentPosAsPureQuat * _cachedTrackedRbRotationQuat;


				/* Finally, once we're in object space coordinate, reapply the correct transformation to go back to the world coordinate (with correct transformation) */

				// 1- Apply the rotation
				currentPosAsPureQuat = currentQuatRotation * currentPosAsPureQuat * currentConjugateQuatRotation;

				// 2- Apply the translation
				const Storm::Vector3 newPPosition = currentPosAsPureQuat.vec() + currentPosition;

				currentPVelocity = newPPosition - currentPPosition;
				currentPVelocity /= deltaTimeInSec;

				currentPPosition = newPPosition;
			});

			this->setParticleSystemPosition(currentPosition);
			_cachedTrackedRbRotationQuat = currentQuatRotation;

			// The force is for the first frame, where we set the position to the position in scene.
			// The velocity mustn't be changed because it is a artificial move (not a physic move) from object space to world space.
			if (force || deltaTimeInSec == 0.f)
			{
				Storm::runParallel(_velocity, [&](Storm::Vector3 &currentPVelocity)
				{
					currentPVelocity.setZero();
				});
			}
			else
			{
				this->setParticleSystemTotalForce(physicsMgr.getForceOnPhysicalBody(_particleSystemIndex, deltaTimeInSec));
			}
		}
		else if (_velocityDirtyInternal)
		{
			_velocityDirtyInternal = false;

			Storm::runParallel(_velocity, [&](Storm::Vector3 &currentPVelocity)
			{
				currentPVelocity.setZero();
			});
		}
	}
}

void Storm::RigidBodyParticleSystem::revertToCurrentTimestep(const std::vector<std::unique_ptr<Storm::IBlower>> &)
{
	if (!_isStatic)
	{
		Storm::runParallel(_force, [this](Storm::Vector3 &force, const std::size_t currentPIndex) 
		{
			force.setZero();
			_tmpPressureForce[currentPIndex].setZero();
			_tmpViscosityForce[currentPIndex].setZero();
		});
	}
}
