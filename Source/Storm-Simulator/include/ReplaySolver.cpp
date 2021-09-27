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
#define STORM_HIJACKED_TYPE Storm::Vector3
#	include "VectHijack.h"
#undef STORM_HIJACKED_TYPE

#include "InstructionSet.h"
#include "SIMDUsageMode.h"

#include <future>


namespace
{
	auto makeSSELerpArrayLambda(float coefficient)
	{
		return [coefficient]<class ValueType>(const std::vector<ValueType> &inBeforeArray, const std::vector<ValueType> &inAfterArray, std::vector<ValueType> &outResultArray)
		{
			const __m128 coeff = _mm_set1_ps(coefficient);

			if constexpr (std::is_same_v<ValueType, float> || (std::is_same_v<ValueType, Storm::Vector3> && std::is_same_v<float, Storm::Vector3::Scalar>))
			{
				enum : std::size_t { k_shift = 4 };
				const std::size_t arrayCount = outResultArray.size() * (sizeof(ValueType) / sizeof(float));
				const std::size_t remaining = arrayCount % k_shift;
				
				auto fastLerp = [
					&coeff,
					beforePtr = reinterpret_cast<const float*>(inBeforeArray.data()),
					afterPtr = reinterpret_cast<const float*>(inAfterArray.data()),
					resultPtr = reinterpret_cast<float*>(outResultArray.data())
				](const std::size_t iter)
				{
					const __m128 tmpBefore = _mm_loadu_ps(beforePtr + iter);
					const __m128 tmpAfter = _mm_loadu_ps(afterPtr + iter);

					_mm_storeu_ps(resultPtr + iter, _mm_add_ps(tmpBefore, _mm_mul_ps(_mm_sub_ps(tmpAfter, tmpBefore), coeff)));
				};

				if (remaining > 0)
				{
					const std::size_t alignedArrayCount = arrayCount - k_shift;
					for (std::size_t iter = 0; iter < alignedArrayCount; iter += k_shift)
					{
						fastLerp(iter);
					}

					fastLerp(alignedArrayCount);
				}
				else
				{
					for (std::size_t iter = 0; iter < arrayCount; iter += k_shift)
					{
						fastLerp(iter);
					}
				}
			}
			else
			{
				STORM_COMPILE_ERROR("Unhandled lerp mode");
			}
		};
	}

	auto makeAVX512LerpArrayLambda(float coefficient)
	{
		return [coefficient]<class ValueType>(const std::vector<ValueType> &inBeforeArray, const std::vector<ValueType> &inAfterArray, std::vector<ValueType> &outResultArray)
		{
			const __m512 coeff = _mm512_set1_ps(coefficient);

			if constexpr (std::is_same_v<ValueType, float> || (std::is_same_v<ValueType, Storm::Vector3> && std::is_same_v<float, Storm::Vector3::Scalar>))
			{
				enum : std::size_t { k_shift = 16 };
				const std::size_t arrayCount = outResultArray.size() * (sizeof(ValueType) / sizeof(float));
				const std::size_t remaining = arrayCount % k_shift;

				auto fastLerp = [
					&coeff,
					beforePtr = reinterpret_cast<const float*>(inBeforeArray.data()),
					afterPtr = reinterpret_cast<const float*>(inAfterArray.data()),
					resultPtr = reinterpret_cast<float*>(outResultArray.data())
				](const std::size_t iter)
				{
					const __m512 tmpBefore = _mm512_loadu_ps(beforePtr + iter);
					const __m512 tmpAfter = _mm512_loadu_ps(afterPtr + iter);

					_mm512_storeu_ps(resultPtr + iter, _mm512_add_ps(tmpBefore, _mm512_mul_ps(_mm512_sub_ps(tmpAfter, tmpBefore), coeff)));
				};

				if (remaining > 0)
				{
					const std::size_t alignedArrayCount = arrayCount - k_shift;
					for (std::size_t iter = 0; iter < alignedArrayCount; iter += k_shift)
					{
						fastLerp(iter);
					}

					fastLerp(alignedArrayCount);
				}
				else
				{
					for (std::size_t iter = 0; iter < arrayCount; iter += k_shift)
					{
						fastLerp(iter);
					}
				}
			}
			else
			{
				STORM_COMPILE_ERROR("Unhandled lerp mode");
			}
		};
	}
	
	auto makeSSECpyArrayLambda()
	{
		return []<class ValueType>(const std::vector<ValueType> &srcArray, std::vector<ValueType> &dstArray)
		{
			enum : std::size_t { k_shift = 4 };

			const std::size_t arrayCount = dstArray.size() * (sizeof(ValueType) / sizeof(float));
			const std::size_t remaining = arrayCount % k_shift;
			
			auto fastCpy = [srcPtr = reinterpret_cast<const float*>(srcArray.data()), dstPtr = reinterpret_cast<float*>(dstArray.data())](const std::size_t iter)
			{
				_mm_storeu_ps(dstPtr + iter, _mm_loadu_ps(srcPtr + iter));
			};

			if (remaining > 0)
			{
				const std::size_t alignedArrayCount = arrayCount - k_shift;
				for (std::size_t iter = 0; iter < alignedArrayCount; iter += k_shift)
				{
					fastCpy(iter);
				}

				fastCpy(alignedArrayCount);
			}
			else
			{
				for (std::size_t iter = 0; iter < arrayCount; iter += k_shift)
				{
					fastCpy(iter);
				}
			}
		};
	}
	
	auto makeAVX512CpyArrayLambda()
	{
		return[]<class ValueType>(const std::vector<ValueType> &srcArray, std::vector<ValueType> &dstArray)
		{
			enum : std::size_t { k_shift = 16 };

			const std::size_t arrayCount = srcArray.size() * (sizeof(ValueType) / sizeof(float));
			const std::size_t remaining = arrayCount % k_shift;
			
			auto fastCpy = [srcPtr = reinterpret_cast<const float*>(srcArray.data()), dstPtr = reinterpret_cast<float*>(dstArray.data())](const std::size_t iter)
			{
				_mm512_storeu_ps(dstPtr + iter, _mm512_loadu_ps(srcPtr + iter));
			};

			if (remaining > 0)
			{
				const std::size_t alignedArrayCount = arrayCount - k_shift;
				for (std::size_t iter = 0; iter < alignedArrayCount; iter += k_shift)
				{
					fastCpy(iter);
				}

				fastCpy(alignedArrayCount);
			}
			else
			{
				for (std::size_t iter = 0; iter < arrayCount; iter += k_shift)
				{
					fastCpy(iter);
				}
			}
		};
	}

	__forceinline void lerp(const Storm::Vector3 &vectBefore, const Storm::Vector3 &vectAfter, const float coeff, Storm::Vector3 &result)
	{
		result = vectBefore + (vectAfter - vectBefore) * coeff;
	}

	__forceinline void lerp(const float valBefore, const float valAfter, const float coeff, float &result)
	{
		result = std::lerp(valBefore, valAfter, coeff);
	}


	template<bool remap, Storm::SIMDUsageMode simdMode>
	void lerpParticleSystemsFrames(Storm::ParticleSystemContainer &particleSystems, Storm::SerializeRecordPendingData &frameBefore, Storm::SerializeRecordPendingData &frameAfter, const float coefficient)
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
			std::vector<Storm::Vector3> &allDragForce = currentPSystem.getTemporaryDragForces();
			std::vector<Storm::Vector3> &allDynamicQForce = currentPSystem.getTemporaryBernoulliDynamicPressureForces();
			std::vector<Storm::Vector3> &allNoStickForce = currentPSystem.getTemporaryNoStickForces();


#define STORM_LAUNCH_LERP_ARRAY_FUTURE(memberName, resultArray) std::async(std::launch::async, [&lerpArray, &frameBeforeElements, &frameAfterElements, &resultArray]() { lerpArray(frameAfterElements.memberName, frameAfterElements.memberName, resultArray); })

			if (currentPSystem.isFluids())
			{
				Storm::FluidParticleSystem &currentPSystemAsFluid = static_cast<Storm::FluidParticleSystem &>(currentPSystem);
				std::vector<float> &allDensities = currentPSystemAsFluid.getDensities();
				std::vector<float> &allPressures = currentPSystemAsFluid.getPressures();

#define STORM_MAKE_PARALLEL_FLUID_LERPS															\
	std::future<void> lerpsComputators[] =														\
	{																							\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_positions, allPositions),								\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_velocities, allVelocities),								\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_forces, allForces),										\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_densities, allDensities),								\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_pressures, allPressures),								\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_pressureComponentforces, allPressureForce),				\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_viscosityComponentforces, allViscosityForce),			\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_dragComponentforces, allDragForce),						\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_dynamicPressureQForces, allDynamicQForce),				\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_noStickForces, allNoStickForce),						\
	}

				if constexpr (simdMode == Storm::SIMDUsageMode::AVX512)
				{
					auto lerpArray = makeAVX512LerpArrayLambda(coefficient);
					STORM_MAKE_PARALLEL_FLUID_LERPS;
				}
				else if constexpr (simdMode == Storm::SIMDUsageMode::SSE)
				{
					auto lerpArray = makeSSELerpArrayLambda(coefficient);
					STORM_MAKE_PARALLEL_FLUID_LERPS;
				}
				else
				{
					Storm::runParallel(frameBeforeElements._positions, [&](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
					{
						lerp(currentPPosition, frameAfterElements._positions[currentPIndex], coefficient, allPositions[currentPIndex]);
						lerp(frameBeforeElements._velocities[currentPIndex], frameAfterElements._velocities[currentPIndex], coefficient, allVelocities[currentPIndex]);
						lerp(frameBeforeElements._forces[currentPIndex], frameAfterElements._forces[currentPIndex], coefficient, allForces[currentPIndex]);
						lerp(frameBeforeElements._densities[currentPIndex], frameAfterElements._densities[currentPIndex], coefficient, allDensities[currentPIndex]);
						lerp(frameBeforeElements._pressures[currentPIndex], frameAfterElements._pressures[currentPIndex], coefficient, allPressures[currentPIndex]);
						lerp(frameBeforeElements._pressureComponentforces[currentPIndex], frameAfterElements._pressureComponentforces[currentPIndex], coefficient, allPressureForce[currentPIndex]);
						lerp(frameBeforeElements._viscosityComponentforces[currentPIndex], frameAfterElements._viscosityComponentforces[currentPIndex], coefficient, allViscosityForce[currentPIndex]);
						lerp(frameBeforeElements._dragComponentforces[currentPIndex], frameAfterElements._dragComponentforces[currentPIndex], coefficient, allDragForce[currentPIndex]);
						lerp(frameBeforeElements._dynamicPressureQForces[currentPIndex], frameAfterElements._dynamicPressureQForces[currentPIndex], coefficient, allDynamicQForce[currentPIndex]);
						lerp(frameBeforeElements._noStickForces[currentPIndex], frameAfterElements._noStickForces[currentPIndex], coefficient, allNoStickForce[currentPIndex]);
					});
				}
#undef STORM_MAKE_PARALLEL_FLUID_LERPS

			}
			else
			{
				Storm::RigidBodyParticleSystem &currentPSystemAsRb = static_cast<Storm::RigidBodyParticleSystem &>(currentPSystem);
				std::vector<float> &allVolumes = currentPSystemAsRb.getVolumes();
				std::vector<Storm::Vector3> &allNormals = currentPSystemAsRb.getNormals();

#define STORM_MAKE_PARALLEL_RB_LERPS															\
	std::future<void> lerpsComputators[] =														\
	{																							\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_positions, allPositions),								\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_velocities, allVelocities),								\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_forces, allForces),										\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_volumes, allVolumes),									\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_normals, allNormals),									\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_pressureComponentforces, allPressureForce),				\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_viscosityComponentforces, allViscosityForce),			\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_dragComponentforces, allDragForce),						\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_dynamicPressureQForces, allDynamicQForce),				\
		STORM_LAUNCH_LERP_ARRAY_FUTURE(_noStickForces, allNoStickForce),						\
	}
				if constexpr (simdMode == Storm::SIMDUsageMode::AVX512)
				{
					auto lerpArray = makeAVX512LerpArrayLambda(coefficient);
					STORM_MAKE_PARALLEL_RB_LERPS;
				}
				else if constexpr (simdMode == Storm::SIMDUsageMode::SSE)
				{
					auto lerpArray = makeSSELerpArrayLambda(coefficient);
					STORM_MAKE_PARALLEL_RB_LERPS;
				}
				else
				{
					Storm::runParallel(frameBeforeElements._positions, [&](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
					{
						lerp(currentPPosition, frameAfterElements._positions[currentPIndex], coefficient, allPositions[currentPIndex]);
						lerp(frameBeforeElements._velocities[currentPIndex], frameAfterElements._velocities[currentPIndex], coefficient, allVelocities[currentPIndex]);
						lerp(frameBeforeElements._forces[currentPIndex], frameAfterElements._forces[currentPIndex], coefficient, allForces[currentPIndex]);
						lerp(frameBeforeElements._volumes[currentPIndex], frameAfterElements._volumes[currentPIndex], coefficient, allVolumes[currentPIndex]);
						lerp(frameBeforeElements._normals[currentPIndex], frameAfterElements._normals[currentPIndex], coefficient, allNormals[currentPIndex]);
						lerp(frameBeforeElements._pressureComponentforces[currentPIndex], frameAfterElements._pressureComponentforces[currentPIndex], coefficient, allPressureForce[currentPIndex]);
						lerp(frameBeforeElements._viscosityComponentforces[currentPIndex], frameAfterElements._viscosityComponentforces[currentPIndex], coefficient, allViscosityForce[currentPIndex]);
						lerp(frameBeforeElements._dragComponentforces[currentPIndex], frameAfterElements._dragComponentforces[currentPIndex], coefficient, allDragForce[currentPIndex]);
						lerp(frameBeforeElements._dynamicPressureQForces[currentPIndex], frameAfterElements._dynamicPressureQForces[currentPIndex], coefficient, allDynamicQForce[currentPIndex]);
						lerp(frameBeforeElements._noStickForces[currentPIndex], frameAfterElements._noStickForces[currentPIndex], coefficient, allNoStickForce[currentPIndex]);
					});
#undef STORM_MAKE_PARALLEL_RB_LERPS
				}
			}
#undef STORM_LAUNCH_LERP_ARRAY_FUTURE
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
					Storm::throwException<Storm::Exception>("Cannot find the constraints with id " + std::to_string(frameAfterElements._id) + " inside recorded constraints frame data");
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

	template<Storm::SIMDUsageMode simdMode>
	void fillRecordFromSystemsImpl(const bool pushStatics, const Storm::ParticleSystemContainer &particleSystems, Storm::SerializeRecordPendingData &currentFrameData)
	{
		enum { k_pSystemArrayMaxCount = 10 };
		const std::size_t particleSystemCount = particleSystems.size();

		currentFrameData._particleSystemElements.reserve(particleSystemCount);
		std::vector<std::future<void>> fillerFuturesExecutor;
		fillerFuturesExecutor.reserve(particleSystemCount * k_pSystemArrayMaxCount);

		const auto sseCpyLambda = makeSSECpyArrayLambda();
		const auto avx512CpyLambda = makeAVX512CpyArrayLambda();

		for (const auto &particleSystemPair : particleSystems)
		{
			const Storm::ParticleSystem &pSystemRef = *particleSystemPair.second;

			if (!pSystemRef.isStatic() || pushStatics)
			{
				Storm::SerializeRecordParticleSystemData &framePSystemElementData = currentFrameData._particleSystemElements.emplace_back();

				framePSystemElementData._systemId = particleSystemPair.first;

#define STORM_COPY_ARRAYS(cpyLambda, memberName, srcArray)																								\
	fillerFuturesExecutor.emplace_back(std::async(std::launch::async, [&cpyLambda, &dst = framePSystemElementData.memberName, &src = srcArray]()		\
	{																																					\
		setNumUninitializedIfCountMismatch(dst, src.size());																							\
		cpyLambda(src, dst);																															\
	}))

#define STORM_MAKE_SIMPLE_COPY_ARRAY(memberName, srcArray)																				\
	fillerFuturesExecutor.emplace_back(std::async(std::launch::async, [&dst = framePSystemElementData.memberName, &src = srcArray]()	\
	{																																	\
		dst = src;																														\
	}))

				if (pSystemRef.isFluids())
				{
					const Storm::FluidParticleSystem &pSystemRefAsFluid = static_cast<const Storm::FluidParticleSystem &>(pSystemRef);

					if constexpr (simdMode == Storm::SIMDUsageMode::AVX512)
					{
						STORM_COPY_ARRAYS(avx512CpyLambda, _densities, pSystemRefAsFluid.getDensities());
						STORM_COPY_ARRAYS(avx512CpyLambda, _pressures, pSystemRefAsFluid.getPressures());
					}
					else if constexpr (simdMode == Storm::SIMDUsageMode::SSE)
					{
						STORM_COPY_ARRAYS(sseCpyLambda, _densities, pSystemRefAsFluid.getDensities());
						STORM_COPY_ARRAYS(sseCpyLambda, _pressures, pSystemRefAsFluid.getPressures());
					}
					else
					{
						STORM_MAKE_SIMPLE_COPY_ARRAY(_densities, pSystemRefAsFluid.getDensities());
						STORM_MAKE_SIMPLE_COPY_ARRAY(_pressures, pSystemRefAsFluid.getPressures());
					}
				}
				else
				{
					const Storm::RigidBodyParticleSystem &pSystemRefAsRb = static_cast<const Storm::RigidBodyParticleSystem &>(pSystemRef);
					if constexpr (simdMode == Storm::SIMDUsageMode::AVX512)
					{
						STORM_COPY_ARRAYS(avx512CpyLambda, _normals, pSystemRefAsRb.getNormals());
						STORM_COPY_ARRAYS(avx512CpyLambda, _volumes, pSystemRefAsRb.getVolumes());
					}
					else if constexpr (simdMode == Storm::SIMDUsageMode::SSE)
					{
						STORM_COPY_ARRAYS(sseCpyLambda, _normals, pSystemRefAsRb.getNormals());
						STORM_COPY_ARRAYS(sseCpyLambda, _volumes, pSystemRefAsRb.getVolumes());
					}
					else
					{
						STORM_MAKE_SIMPLE_COPY_ARRAY(_normals, pSystemRefAsRb.getNormals());
						STORM_MAKE_SIMPLE_COPY_ARRAY(_volumes, pSystemRefAsRb.getVolumes());
					}

					framePSystemElementData._pSystemPosition = pSystemRefAsRb.getRbPosition();
					framePSystemElementData._pSystemGlobalForce = pSystemRefAsRb.getRbTotalForce();
				}

				if constexpr (simdMode == Storm::SIMDUsageMode::AVX512)
				{
					STORM_COPY_ARRAYS(avx512CpyLambda, _positions, pSystemRef.getPositions());
					STORM_COPY_ARRAYS(avx512CpyLambda, _velocities, pSystemRef.getVelocity());
					STORM_COPY_ARRAYS(avx512CpyLambda, _forces, pSystemRef.getForces());
					STORM_COPY_ARRAYS(avx512CpyLambda, _pressureComponentforces, pSystemRef.getTemporaryPressureForces());
					STORM_COPY_ARRAYS(avx512CpyLambda, _viscosityComponentforces, pSystemRef.getTemporaryViscosityForces());
					STORM_COPY_ARRAYS(avx512CpyLambda, _dragComponentforces, pSystemRef.getTemporaryDragForces());
					STORM_COPY_ARRAYS(avx512CpyLambda, _dynamicPressureQForces, pSystemRef.getTemporaryBernoulliDynamicPressureForces());
					STORM_COPY_ARRAYS(avx512CpyLambda, _noStickForces, pSystemRef.getTemporaryNoStickForces());
				}
				else if constexpr (simdMode == Storm::SIMDUsageMode::SSE)
				{
					STORM_COPY_ARRAYS(sseCpyLambda, _positions, pSystemRef.getPositions());
					STORM_COPY_ARRAYS(sseCpyLambda, _velocities, pSystemRef.getVelocity());
					STORM_COPY_ARRAYS(sseCpyLambda, _forces, pSystemRef.getForces());
					STORM_COPY_ARRAYS(sseCpyLambda, _pressureComponentforces, pSystemRef.getTemporaryPressureForces());
					STORM_COPY_ARRAYS(sseCpyLambda, _viscosityComponentforces, pSystemRef.getTemporaryViscosityForces());
					STORM_COPY_ARRAYS(sseCpyLambda, _dragComponentforces, pSystemRef.getTemporaryDragForces());
					STORM_COPY_ARRAYS(sseCpyLambda, _dynamicPressureQForces, pSystemRef.getTemporaryBernoulliDynamicPressureForces());
					STORM_COPY_ARRAYS(sseCpyLambda, _noStickForces, pSystemRef.getTemporaryNoStickForces());
				}
				else
				{
					STORM_MAKE_SIMPLE_COPY_ARRAY(_positions, pSystemRef.getPositions());
					STORM_MAKE_SIMPLE_COPY_ARRAY(_velocities, pSystemRef.getVelocity());
					STORM_MAKE_SIMPLE_COPY_ARRAY(_forces, pSystemRef.getForces());
					STORM_MAKE_SIMPLE_COPY_ARRAY(_pressureComponentforces, pSystemRef.getTemporaryPressureForces());
					STORM_MAKE_SIMPLE_COPY_ARRAY(_viscosityComponentforces, pSystemRef.getTemporaryViscosityForces());
					STORM_MAKE_SIMPLE_COPY_ARRAY(_dragComponentforces, pSystemRef.getTemporaryDragForces());
					STORM_MAKE_SIMPLE_COPY_ARRAY(_dynamicPressureQForces, pSystemRef.getTemporaryBernoulliDynamicPressureForces());
					STORM_MAKE_SIMPLE_COPY_ARRAY(_noStickForces, pSystemRef.getTemporaryNoStickForces());
				}

#undef STORM_COPY_ARRAYS
#undef STORM_MAKE_SIMPLE_COPY_ARRAY
			}
		}

		fillerFuturesExecutor.clear();
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
		particleSystem.setNormals(std::move(currentFrameElement._normals));
		particleSystem.setForces(std::move(currentFrameElement._forces));
		particleSystem.setTmpPressureForces(std::move(currentFrameElement._pressureComponentforces));
		particleSystem.setTmpViscosityForces(std::move(currentFrameElement._viscosityComponentforces));
		particleSystem.setTmpDragForces(std::move(currentFrameElement._dragComponentforces));
		particleSystem.setTmpBernoulliDynamicPressureForces(std::move(currentFrameElement._dynamicPressureQForces));
		particleSystem.setTmpNoStickForces(std::move(currentFrameElement._noStickForces));
	}
}

void Storm::ReplaySolver::transferFrameToParticleSystem_copy(Storm::ParticleSystemContainer &particleSystems, Storm::SerializeRecordPendingData &frameFrom)
{
	const bool useSIMD = Storm::InstructionSet::SSE() && Storm::InstructionSet::SSE2();
	const bool useAVX512 = useSIMD && Storm::InstructionSet::AVX512F();

	for (auto &frameElement : frameFrom._particleSystemElements)
	{
		Storm::ParticleSystem &currentPSystem = *particleSystems[frameElement._systemId];
		std::vector<Storm::Vector3> &allPositions = currentPSystem.getPositions();
		std::vector<Storm::Vector3> &allVelocities = currentPSystem.getVelocity();
		std::vector<Storm::Vector3> &allForces = currentPSystem.getForces();
		std::vector<Storm::Vector3> &allPressureForce = currentPSystem.getTemporaryPressureForces();
		std::vector<Storm::Vector3> &allViscosityForce = currentPSystem.getTemporaryViscosityForces();
		std::vector<Storm::Vector3> &allDragForce = currentPSystem.getTemporaryDragForces();
		std::vector<Storm::Vector3> &allDynamicQForce = currentPSystem.getTemporaryBernoulliDynamicPressureForces();
		std::vector<Storm::Vector3> &allNoStickForce = currentPSystem.getTemporaryNoStickForces();

		if (currentPSystem.isFluids())
		{
			Storm::FluidParticleSystem &currentPSystemAsFluid = static_cast<Storm::FluidParticleSystem &>(currentPSystem);
			std::vector<float> &allDensities = currentPSystemAsFluid.getDensities();
			std::vector<float> &allPressures = currentPSystemAsFluid.getPressures();

			const std::size_t framePCount = frameElement._positions.size();
			setNumUninitializedIfCountMismatch(frameElement._densities, framePCount);
			setNumUninitializedIfCountMismatch(frameElement._pressures, framePCount);

			if (useSIMD)
			{
#define STORM_LAUNCH_CPY_ARRAY_FUTURE(memberName, resultArray) std::async(std::launch::async, [&cpyArray, &frameElement, &resultArray]() { cpyArray(frameElement.memberName, resultArray); })

#define STORM_MAKE_PARALLEL_FLUID_CPY															\
	std::future<void> cpysComputators[] =														\
	{																							\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_positions, allPositions),								\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_velocities, allVelocities),								\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_forces, allForces),										\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_densities, allDensities),								\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_pressures, allPressures),								\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_pressureComponentforces, allPressureForce),				\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_viscosityComponentforces, allViscosityForce),			\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_dragComponentforces, allDragForce),						\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_dynamicPressureQForces, allDynamicQForce),				\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_noStickForces, allNoStickForce),							\
	}

				if (useAVX512)
				{
					auto cpyArray = makeAVX512CpyArrayLambda();
					STORM_MAKE_PARALLEL_FLUID_CPY;
				}
				else
				{
					auto cpyArray = makeSSECpyArrayLambda();
					STORM_MAKE_PARALLEL_FLUID_CPY;
				}
#undef STORM_MAKE_PARALLEL_FLUID_CPY
			}
			else
			{
				Storm::runParallel(frameElement._positions, [&](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
				{
					allPositions[currentPIndex] = currentPPosition;
					allVelocities[currentPIndex] = frameElement._velocities[currentPIndex];
					allForces[currentPIndex] = frameElement._forces[currentPIndex];
					allDensities[currentPIndex] = frameElement._densities[currentPIndex];
					allPressures[currentPIndex] = frameElement._pressures[currentPIndex];
					allPressureForce[currentPIndex] = frameElement._pressureComponentforces[currentPIndex];
					allViscosityForce[currentPIndex] = frameElement._viscosityComponentforces[currentPIndex];
					allDragForce[currentPIndex] = frameElement._dragComponentforces[currentPIndex];
					allDynamicQForce[currentPIndex] = frameElement._dynamicPressureQForces[currentPIndex];
					allNoStickForce[currentPIndex] = frameElement._noStickForces[currentPIndex];
				});
			}
		}
		else
		{
			Storm::RigidBodyParticleSystem &currentPSystemAsRb = static_cast<Storm::RigidBodyParticleSystem &>(currentPSystem);
			std::vector<float> &allVolumes = currentPSystemAsRb.getVolumes();
			std::vector<Storm::Vector3> &allNormals = currentPSystemAsRb.getNormals();

			if (useSIMD)
			{
#define STORM_MAKE_PARALLEL_RB_CPY																\
	std::future<void> cpysComputators[] =														\
	{																							\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_positions, allPositions),								\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_velocities, allVelocities),								\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_forces, allForces),										\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_volumes, allVolumes),									\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_normals, allNormals),									\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_pressureComponentforces, allPressureForce),				\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_viscosityComponentforces, allViscosityForce),			\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_dragComponentforces, allDragForce),						\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_dynamicPressureQForces, allDynamicQForce),				\
		STORM_LAUNCH_CPY_ARRAY_FUTURE(_noStickForces, allNoStickForce),							\
	}

				if (useAVX512)
				{
					auto cpyArray = makeAVX512CpyArrayLambda();
					STORM_MAKE_PARALLEL_RB_CPY;
				}
				else
				{
					auto cpyArray = makeSSECpyArrayLambda();
					STORM_MAKE_PARALLEL_RB_CPY;
				}
#undef STORM_MAKE_PARALLEL_RB_CPY

			}
			else
			{
				Storm::runParallel(frameElement._positions, [&](const Storm::Vector3 &currentPPosition, const std::size_t currentPIndex)
				{
					allPositions[currentPIndex] = currentPPosition;
					allVelocities[currentPIndex] = frameElement._velocities[currentPIndex];
					allForces[currentPIndex] = frameElement._forces[currentPIndex];
					allVolumes[currentPIndex] = frameElement._volumes[currentPIndex];
					allNormals[currentPIndex] = frameElement._normals[currentPIndex];
					allPressureForce[currentPIndex] = frameElement._pressureComponentforces[currentPIndex];
					allViscosityForce[currentPIndex] = frameElement._viscosityComponentforces[currentPIndex];
					allDragForce[currentPIndex] = frameElement._dragComponentforces[currentPIndex];
					allDynamicQForce[currentPIndex] = frameElement._dynamicPressureQForces[currentPIndex];
					allNoStickForce[currentPIndex] = frameElement._noStickForces[currentPIndex];
				});
			}

			currentPSystem.setParticleSystemPosition(frameElement._pSystemPosition);
			currentPSystem.setParticleSystemTotalForce(frameElement._pSystemGlobalForce);
		}

#undef STORM_LAUNCH_CPY_ARRAY_FUTURE
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
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
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
		const float coefficient = 1.f - ((frameAfter._physicsTime - currentTime) / frameDiffTime);

		const bool useSIMD = Storm::InstructionSet::SSE() && Storm::InstructionSet::SSE2();
		const bool useAVX512 = useSIMD && Storm::InstructionSet::AVX512F();

		// The first frame is different because it contains static rigid bodies while the other frames doesn't contains them.
		// It results in a mismatch of index when reading the frames element by their index in the array. Therefore, we should remap in case of a size mismatch.
		if (frameBefore._particleSystemElements.size() == frameAfter._particleSystemElements.size())
		{
			if (useSIMD)
			{
				if (useAVX512)
				{
					lerpParticleSystemsFrames<false, Storm::SIMDUsageMode::AVX512>(particleSystems, frameBefore, frameAfter, coefficient);
				}
				else
				{
					lerpParticleSystemsFrames<false, Storm::SIMDUsageMode::SSE>(particleSystems, frameBefore, frameAfter, coefficient);
				}
			}
			else
			{
				lerpParticleSystemsFrames<false, Storm::SIMDUsageMode::SISD>(particleSystems, frameBefore, frameAfter, coefficient);
			}
		}
		else
		{
			if (useSIMD)
			{
				if (useAVX512)
				{
					lerpParticleSystemsFrames<true, Storm::SIMDUsageMode::AVX512>(particleSystems, frameBefore, frameAfter, coefficient);
				}
				else
				{
					lerpParticleSystemsFrames<true, Storm::SIMDUsageMode::SSE>(particleSystems, frameBefore, frameAfter, coefficient);
				}
			}
			else
			{
				lerpParticleSystemsFrames<true, Storm::SIMDUsageMode::SISD>(particleSystems, frameBefore, frameAfter, coefficient);
			}
		}

		lerpConstraintsFrames(frameBefore, frameAfter, coefficient, outFrameConstraintData);

		timeMgr.setCurrentPhysicsElapsedTime(nextFrameTime);
	}

	return true;
}

void Storm::ReplaySolver::fillRecordFromSystems(const bool pushStatics, const Storm::ParticleSystemContainer &particleSystems, Storm::SerializeRecordPendingData &currentFrameData)
{
	const bool useSIMD = Storm::InstructionSet::SSE() && Storm::InstructionSet::SSE2();
	const bool useAVX512 = useSIMD && Storm::InstructionSet::AVX512F();

	if (useSIMD)
	{
		if (useAVX512)
		{
			fillRecordFromSystemsImpl<Storm::SIMDUsageMode::AVX512>(pushStatics, particleSystems, currentFrameData);
		}
		else
		{
			fillRecordFromSystemsImpl<Storm::SIMDUsageMode::SSE>(pushStatics, particleSystems, currentFrameData);
		}
	}
	else
	{
		fillRecordFromSystemsImpl<Storm::SIMDUsageMode::SISD>(pushStatics, particleSystems, currentFrameData);
	}
}
