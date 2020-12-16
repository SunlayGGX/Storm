#include "ReplaySolver.h"

#include "SingletonHolder.h"
#include "ITimeManager.h"
#include "ISerializerManager.h"

#include "ParticleSystem.h"
#include "FluidParticleSystem.h"
#include "RigidBodyParticleSystem.h"

#include "SerializeRecordPendingData.h"
#include "SerializeRecordParticleSystemData.h"
#include "SerializeRecordContraintsData.h"

#include "RunnerHelper.h"

#define STORM_HIJACKED_TYPE float
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE


namespace
{
#if STORM_USE_INTRINSICS
	void lerp(const Storm::Vector3 &vectBefore, const Storm::Vector3 &vectAfter, const __m128 &coeff, Storm::Vector3 &result)
	{
		const __m128 tmpBefore = _mm_loadu_ps(&vectBefore[0]);
		const __m128 tmpAfter = _mm_loadu_ps(&vectAfter[0]);

		const __m128 res = _mm_add_ps(tmpBefore, _mm_mul_ss(_mm_sub_ps(tmpAfter, tmpBefore), coeff));

		result.x() = res.m128_f32[0];
		result.y() = res.m128_f32[1];
		result.z() = res.m128_f32[2];
	}

	void lerp(const float valBefore, const float valAfter, const __m128 &coeff, float &result)
	{
		result = std::lerp(valBefore, valAfter, coeff.m128_f32[0]);
	}
#else
	void lerp(const Storm::Vector3 &vectBefore, const Storm::Vector3 &vectAfter, const float coeff, Storm::Vector3 &result)
	{
		result = vectBefore + (vectAfter - vectBefore) * coeff;
	}

	void lerp(const float valBefore, const float valAfter, const float coeff, float &result)
	{
		result = std::lerp(valBefore, valAfter, coeff);
	}
#endif


	template<bool remap, class CoefficientType>
	void lerpParticleSystemsFrames(Storm::ParticleSystemContainer &particleSystems, Storm::SerializeRecordPendingData &frameBefore, Storm::SerializeRecordPendingData &frameAfter, const CoefficientType &coefficient)
	{
		Storm::Vector3 tmp;

		const std::size_t frameElementCount = frameAfter._particleSystemElements.size();
		for (std::size_t iter = 0; iter < frameElementCount; ++iter)
		{
			const Storm::SerializeRecordParticleSystemData &frameAfterElements = frameAfter._particleSystemElements[iter];

			const Storm::SerializeRecordParticleSystemData* frameBeforeElementsPtr = &frameBefore._particleSystemElements[iter];
			
			if constexpr (remap)
			{
				if (frameBeforeElementsPtr->_systemId != frameAfterElements._systemId)
				{
					for (const auto &elementsBefore : frameBefore._particleSystemElements)
					{
						if (elementsBefore._systemId == frameAfterElements._systemId)
						{
							frameBeforeElementsPtr = &elementsBefore;
							break;
						}
					}
				}
			}

			const Storm::SerializeRecordParticleSystemData &frameBeforeElements = *frameBeforeElementsPtr;

			Storm::ParticleSystem &currentPSystem = *particleSystems[frameBeforeElements._systemId];

			lerp(frameBeforeElements._pSystemPosition, frameAfterElements._pSystemPosition, coefficient, tmp);
			currentPSystem.setParticleSystemPosition(tmp);

			lerp(frameBeforeElements._pSystemGlobalForce, frameAfterElements._pSystemGlobalForce, coefficient, tmp);
			currentPSystem.setParticleSystemTotalForce(tmp);

			std::vector<Storm::Vector3> &allPositions = currentPSystem.getPositions();
			std::vector<Storm::Vector3> &allVelocities = currentPSystem.getVelocity();
			std::vector<Storm::Vector3> &allForces = currentPSystem.getForces();
			std::vector<Storm::Vector3> &allPressureForce = currentPSystem.getTemporaryPressureForces();
			std::vector<Storm::Vector3> &allViscosityForce = currentPSystem.getTemporaryViscosityForces();

			if (currentPSystem.isFluids())
			{
				Storm::FluidParticleSystem &currentPSystemAsFluid = static_cast<Storm::FluidParticleSystem &>(currentPSystem);
				std::vector<float> &allDensities = currentPSystemAsFluid.getDensities();
				std::vector<float> &allPressures = currentPSystemAsFluid.getPressures();

				Storm::runParallel(frameBeforeElements._positions, [&](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
				{
					lerp(currentPPosition, frameAfterElements._positions[currentPIndex], coefficient, allPositions[currentPIndex]);
					lerp(frameBeforeElements._velocities[currentPIndex], frameAfterElements._velocities[currentPIndex], coefficient, allVelocities[currentPIndex]);
					lerp(frameBeforeElements._forces[currentPIndex], frameAfterElements._forces[currentPIndex], coefficient, allForces[currentPIndex]);
					lerp(frameBeforeElements._densities[currentPIndex], frameAfterElements._densities[currentPIndex], coefficient, allDensities[currentPIndex]);
					lerp(frameBeforeElements._pressures[currentPIndex], frameAfterElements._pressures[currentPIndex], coefficient, allPressures[currentPIndex]);
					lerp(frameBeforeElements._pressureComponentforces[currentPIndex], frameAfterElements._pressureComponentforces[currentPIndex], coefficient, allPressureForce[currentPIndex]);
					lerp(frameBeforeElements._viscosityComponentforces[currentPIndex], frameAfterElements._viscosityComponentforces[currentPIndex], coefficient, allViscosityForce[currentPIndex]);
				});
			}
			else
			{
				Storm::RigidBodyParticleSystem &currentPSystemAsRb = static_cast<Storm::RigidBodyParticleSystem &>(currentPSystem);
				std::vector<float> &allVolumes = currentPSystemAsRb.getVolumes();

				Storm::runParallel(frameBeforeElements._positions, [&](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
				{
					lerp(currentPPosition, frameAfterElements._positions[currentPIndex], coefficient, allPositions[currentPIndex]);
					lerp(frameBeforeElements._velocities[currentPIndex], frameAfterElements._velocities[currentPIndex], coefficient, allVelocities[currentPIndex]);
					lerp(frameBeforeElements._forces[currentPIndex], frameAfterElements._forces[currentPIndex], coefficient, allForces[currentPIndex]);
					lerp(frameBeforeElements._volumes[currentPIndex], frameAfterElements._volumes[currentPIndex], coefficient, allVolumes[currentPIndex]);
					lerp(frameBeforeElements._pressureComponentforces[currentPIndex], frameAfterElements._pressureComponentforces[currentPIndex], coefficient, allPressureForce[currentPIndex]);
					lerp(frameBeforeElements._viscosityComponentforces[currentPIndex], frameAfterElements._viscosityComponentforces[currentPIndex], coefficient, allViscosityForce[currentPIndex]);
				});
			}
		}
	}

	template<class CoefficientType>
	void lerpConstraintsFrames(Storm::SerializeRecordPendingData &frameBefore, Storm::SerializeRecordPendingData &frameAfter, const CoefficientType &coefficient, std::vector<Storm::SerializeRecordContraintsData> &outFrameConstraintData)
	{
		const std::size_t frameConstraintsCount = frameAfter._constraintElements.size();

		outFrameConstraintData.clear();
		outFrameConstraintData.reserve(frameConstraintsCount);

		for (std::size_t iter = 0; iter < frameConstraintsCount; ++iter)
		{
			const Storm::SerializeRecordContraintsData &frameAfterElements = frameAfter._constraintElements[iter];

			const Storm::SerializeRecordContraintsData* frameBeforeElementsPtr = &frameBefore._constraintElements[iter];

			if (frameAfterElements._id != frameBeforeElementsPtr->_id)
			{
				frameBeforeElementsPtr = nullptr;

				for (std::size_t jiter = iter + 1; jiter < frameConstraintsCount; ++jiter)
				{
					const Storm::SerializeRecordContraintsData* current = &frameBefore._constraintElements[jiter];
					if (current->_id == frameAfterElements._id)
					{
						frameBeforeElementsPtr = current;
						break;
					}
				}

				if (!frameBeforeElementsPtr)
				{
					Storm::throwException<std::exception>("Cannot find the constraints with id " + std::to_string(frameAfterElements._id) + " inside recorded constraints frame data");
				}
			}

			const Storm::SerializeRecordContraintsData &frameBeforeElements = *frameBeforeElementsPtr;

			Storm::SerializeRecordContraintsData &currentConstraints = outFrameConstraintData.emplace_back();

			lerp(frameBeforeElements._position1, frameAfterElements._position1, coefficient, currentConstraints._position1);
			lerp(frameBeforeElements._position2, frameAfterElements._position2, coefficient, currentConstraints._position2);
		}
	}

	template<class Type>
	void setNumUninitializedIfCountMismatch(std::vector<Type> &inOutVect, const std::size_t count)
	{
		if (inOutVect.size() < count)
		{
			inOutVect.reserve(count);
			Storm::setNumUninitialized_hijack(inOutVect, Storm::VectorHijacker{ count });
		}
	}
}


void Storm::ReplaySolver::transferFrameToParticleSystem_move(Storm::ParticleSystemContainer &particleSystems, Storm::SerializeRecordPendingData &frameFrom)
{
	for (auto &currentFrameElement : frameFrom._particleSystemElements)
	{
		Storm::ParticleSystem &particleSystem = *particleSystems[currentFrameElement._systemId];
		particleSystem.setParticleSystemPosition(currentFrameElement._pSystemPosition);
		particleSystem.setParticleSystemTotalForce(currentFrameElement._pSystemGlobalForce);
		particleSystem.setPositions(std::move(currentFrameElement._positions));
		particleSystem.setVelocity(std::move(currentFrameElement._velocities));
		particleSystem.setDensities(std::move(currentFrameElement._densities));
		particleSystem.setPressures(std::move(currentFrameElement._pressures));
		particleSystem.setVolumes(std::move(currentFrameElement._volumes));
		particleSystem.setForces(std::move(currentFrameElement._forces));
		particleSystem.setTmpPressureForces(std::move(currentFrameElement._pressureComponentforces));
		particleSystem.setTmpViscosityForces(std::move(currentFrameElement._viscosityComponentforces));
	}
}

void Storm::ReplaySolver::transferFrameToParticleSystem_copy(Storm::ParticleSystemContainer &particleSystems, Storm::SerializeRecordPendingData &frameFrom)
{
	for (auto &frameElement : frameFrom._particleSystemElements)
	{
		Storm::ParticleSystem &currentPSystem = *particleSystems[frameElement._systemId];
		std::vector<Storm::Vector3> &allPositions = currentPSystem.getPositions();
		std::vector<Storm::Vector3> &allVelocities = currentPSystem.getVelocity();
		std::vector<Storm::Vector3> &allForces = currentPSystem.getForces();
		std::vector<Storm::Vector3> &allPressureForce = currentPSystem.getTemporaryPressureForces();
		std::vector<Storm::Vector3> &allViscosityForce = currentPSystem.getTemporaryViscosityForces();

		if (currentPSystem.isFluids())
		{
			Storm::FluidParticleSystem &currentPSystemAsFluid = static_cast<Storm::FluidParticleSystem &>(currentPSystem);
			std::vector<float> &allDensities = currentPSystemAsFluid.getDensities();
			std::vector<float> &allPressures = currentPSystemAsFluid.getPressures();

			const std::size_t framePCount = frameElement._positions.size();
			setNumUninitializedIfCountMismatch(frameElement._densities, framePCount);
			setNumUninitializedIfCountMismatch(frameElement._pressures, framePCount);

			Storm::runParallel(frameElement._positions, [&](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
			{
				allPositions[currentPIndex] = currentPPosition;
				allVelocities[currentPIndex] = frameElement._velocities[currentPIndex];
				allForces[currentPIndex] = frameElement._forces[currentPIndex];
				allDensities[currentPIndex] = frameElement._densities[currentPIndex];
				allPressures[currentPIndex] = frameElement._pressures[currentPIndex];
				allPressureForce[currentPIndex] = frameElement._pressureComponentforces[currentPIndex];
				allViscosityForce[currentPIndex] = frameElement._viscosityComponentforces[currentPIndex];
			});
		}
		else
		{
			Storm::RigidBodyParticleSystem &currentPSystemAsRb = static_cast<Storm::RigidBodyParticleSystem &>(currentPSystem);
			std::vector<float> &allVolumes = currentPSystemAsRb.getVolumes();

			Storm::runParallel(frameElement._positions, [&](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
			{
				allPositions[currentPIndex] = currentPPosition;
				allVelocities[currentPIndex] = frameElement._velocities[currentPIndex];
				allForces[currentPIndex] = frameElement._forces[currentPIndex];
				allVolumes[currentPIndex] = frameElement._volumes[currentPIndex];
				allPressureForce[currentPIndex] = frameElement._pressureComponentforces[currentPIndex];
				allViscosityForce[currentPIndex] = frameElement._viscosityComponentforces[currentPIndex];
			});
		}

		currentPSystem.setParticleSystemPosition(frameElement._pSystemPosition);
		currentPSystem.setParticleSystemTotalForce(frameElement._pSystemGlobalForce);
	}
}

void Storm::ReplaySolver::computeNextRecordTime(float &inOutNextRecordTime, const float currentPhysicsTime, const float recordFps)
{
	inOutNextRecordTime = std::ceilf(currentPhysicsTime * recordFps) / recordFps;

	// If currentPhysicsTime was a multiple of recordFps (first frame, or with extreme bad luck), then inOutNextRecordTime would be equal to the currentPhysicsTime.
	// We need to increase the record time to the next frame time.
	if (inOutNextRecordTime == currentPhysicsTime)
	{
		inOutNextRecordTime += (1.f / recordFps);
	}
}

bool Storm::ReplaySolver::replayCurrentNextFrame(Storm::ParticleSystemContainer &particleSystems, Storm::SerializeRecordPendingData &frameBefore, Storm::SerializeRecordPendingData &frameAfter, const float recordFps, std::vector<Storm::SerializeRecordContraintsData> &outFrameConstraintData)
{
	Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	Storm::ISerializerManager &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();

	float nextFrameTime;

	if (timeMgr.getExpectedFrameFPS() == recordFps) // No need to interpolate. The frame rates matches. We can work only with frameBefore
	{
		if (!serializerMgr.obtainNextFrame(frameBefore))
		{
			return false;
		}

		Storm::ReplaySolver::transferFrameToParticleSystem_move(particleSystems, frameBefore);
		outFrameConstraintData = std::move(frameBefore._constraintElements);

		timeMgr.setCurrentPhysicsElapsedTime(frameBefore._physicsTime);

		Storm::ReplaySolver::computeNextRecordTime(nextFrameTime, frameBefore._physicsTime, recordFps);
	}
	else // The frame rates don't match. We need to interpolate between the frames.
	{
		float currentTime = timeMgr.getCurrentPhysicsElapsedTime();
		Storm::ReplaySolver::computeNextRecordTime(nextFrameTime, currentTime, recordFps);
		currentTime = nextFrameTime;

		while (frameAfter._physicsTime < currentTime)
		{
			frameBefore = std::move(frameAfter);
			if (!serializerMgr.obtainNextFrame(frameAfter))
			{
				return false;
			}
		}

		const float frameDiffTime = frameAfter._physicsTime - frameBefore._physicsTime;

		// Lerp coeff
#if STORM_USE_INTRINSICS
		const __m128 coefficient = _mm_set1_ps(1.f - ((frameAfter._physicsTime - currentTime) / frameDiffTime));
#else
		const float coefficient = 1.f - ((frameAfter._physicsTime - currentTime) / frameDiffTime);
#endif

		// The first frame is different because it contains static rigid bodies while the other frames doesn't contains them.
		// It results in a mismatch of index when reading the frames element by their index in the array. Therefore, we should remap in case of a size mismatch.
		if (frameBefore._particleSystemElements.size() == frameAfter._particleSystemElements.size())
		{
			lerpParticleSystemsFrames<false>(particleSystems, frameBefore, frameAfter, coefficient);
		}
		else
		{
			lerpParticleSystemsFrames<true>(particleSystems, frameBefore, frameAfter, coefficient);
		}

		lerpConstraintsFrames(frameBefore, frameAfter, coefficient, outFrameConstraintData);

		timeMgr.setCurrentPhysicsElapsedTime(nextFrameTime);
	}

	return true;
}
