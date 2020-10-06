#include "ReplaySolver.h"

#include "SingletonHolder.h"
#include "ITimeManager.h"
#include "ISerializerManager.h"

#include "ParticleSystem.h"
#include "SerializeRecordPendingData.h"
#include "RecordConfigData.h"

#include "RunnerHelper.h"


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
#else
	void lerp(const Storm::Vector3 &vectBefore, const Storm::Vector3 &vectAfter, const float coeff, Storm::Vector3 &result)
	{
		result = vectBefore + (vectAfter - vectBefore) * coeff;
	}
#endif
}


void Storm::ReplaySolver::transferFrameToParticleSystem_move(std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &particleSystems, Storm::SerializeRecordPendingData &frameFrom)
{
	for (auto &currentFrameElement : frameFrom._elements)
	{
		Storm::ParticleSystem &particleSystem = *particleSystems[currentFrameElement._systemId];
		particleSystem.setPositions(std::move(currentFrameElement._positions));
		particleSystem.setVelocity(std::move(currentFrameElement._velocities));
		particleSystem.setForces(std::move(currentFrameElement._forces));
		particleSystem.setTmpPressureForces(std::move(currentFrameElement._pressureComponentforces));
		particleSystem.setTmpViscosityForces(std::move(currentFrameElement._viscosityComponentforces));
	}
}

void Storm::ReplaySolver::transferFrameToParticleSystem_copy(std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &particleSystems, const Storm::SerializeRecordPendingData &frameFrom)
{
	for (const auto &frameElement : frameFrom._elements)
	{
		Storm::ParticleSystem &currentPSystem = *particleSystems[frameElement._systemId];
		std::vector<Storm::Vector3> &allPositions = currentPSystem.getPositions();
		std::vector<Storm::Vector3> &allVelocities = currentPSystem.getVelocity();
		std::vector<Storm::Vector3> &allForces = currentPSystem.getForces();
		std::vector<Storm::Vector3> &allPressureForce = currentPSystem.getTemporaryPressureForces();
		std::vector<Storm::Vector3> &allViscosityForce = currentPSystem.getTemporaryViscosityForces();

		Storm::runParallel(frameElement._positions, [&](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
		{
			allPositions[currentPIndex] = currentPPosition;
			allVelocities[currentPIndex] = frameElement._velocities[currentPIndex];
			allForces[currentPIndex] = frameElement._forces[currentPIndex];
			allPressureForce[currentPIndex] = frameElement._pressureComponentforces[currentPIndex];
			allViscosityForce[currentPIndex] = frameElement._viscosityComponentforces[currentPIndex];
		});
	}
}

void Storm::ReplaySolver::computeNextRecordTime(float &inOutNextRecordTime, const float currentPhysicsTime, const Storm::RecordConfigData &recordConfig)
{
	inOutNextRecordTime = std::ceilf(currentPhysicsTime * recordConfig._recordFps) / recordConfig._recordFps;

	// If currentPhysicsTime was a multiple of recordConfig._recordFps (first frame, or with extreme bad luck), then inOutNextRecordTime would be equal to the currentPhysicsTime.
	// We need to increase the record time to the next frame time.
	if (inOutNextRecordTime == currentPhysicsTime)
	{
		inOutNextRecordTime += (1.f / recordConfig._recordFps);
	}
}

bool Storm::ReplaySolver::replayCurrentNextFrame(std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &particleSystems, Storm::SerializeRecordPendingData &frameBefore, Storm::SerializeRecordPendingData &frameAfter, const Storm::RecordConfigData &recordConfig)
{
	Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
	Storm::ISerializerManager &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();

	float nextFrameTime;

	if (timeMgr.getExpectedFrameFPS() == recordConfig._recordFps) // No need to interpolate. The frame rates matches. We can work only with frameBefore
	{
		if (!serializerMgr.obtainNextFrame(frameBefore))
		{
			return false;
		}

		Storm::ReplaySolver::transferFrameToParticleSystem_move(particleSystems, frameBefore);

		Storm::ReplaySolver::computeNextRecordTime(nextFrameTime, frameBefore._physicsTime, recordConfig);
	}
	else // The frame rates don't match. We need to interpolate between the frames.
	{
		float currentTime = timeMgr.getCurrentPhysicsElapsedTime();
		Storm::ReplaySolver::computeNextRecordTime(nextFrameTime, currentTime, recordConfig);
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

		const std::size_t frameElementCount = frameBefore._elements.size();
		for (std::size_t iter = 0; iter < frameElementCount; ++iter)
		{
			const auto &frameBeforeElements = frameBefore._elements[iter];
			const auto &frameAfterElements = frameAfter._elements[iter];

			Storm::ParticleSystem &currentPSystem = *particleSystems[frameBeforeElements._systemId];
			std::vector<Storm::Vector3> &allPositions = currentPSystem.getPositions();
			std::vector<Storm::Vector3> &allVelocities = currentPSystem.getVelocity();
			std::vector<Storm::Vector3> &allForces = currentPSystem.getForces();
			std::vector<Storm::Vector3> &allPressureForce = currentPSystem.getTemporaryPressureForces();
			std::vector<Storm::Vector3> &allViscosityForce = currentPSystem.getTemporaryViscosityForces();

			Storm::runParallel(frameBeforeElements._positions, [&](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
			{
				lerp(currentPPosition, frameAfterElements._positions[currentPIndex], coefficient, allPositions[currentPIndex]);
				lerp(frameBeforeElements._velocities[currentPIndex], frameAfterElements._velocities[currentPIndex], coefficient, allVelocities[currentPIndex]);
				lerp(frameBeforeElements._forces[currentPIndex], frameAfterElements._forces[currentPIndex], coefficient, allForces[currentPIndex]);
				lerp(frameBeforeElements._pressureComponentforces[currentPIndex], frameAfterElements._pressureComponentforces[currentPIndex], coefficient, allPressureForce[currentPIndex]);
				lerp(frameBeforeElements._viscosityComponentforces[currentPIndex], frameAfterElements._viscosityComponentforces[currentPIndex], coefficient, allViscosityForce[currentPIndex]);
			});
		}

		timeMgr.setCurrentPhysicsElapsedTime(nextFrameTime);
	}

	return true;
}
