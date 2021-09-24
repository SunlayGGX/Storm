#pragma once

#include "ParticleSystemContainer.h"


namespace Storm
{
	namespace details
	{
		template<class VectOrIntrinsType, class NeighborhoodArray, class RawKernelFunc, class GradKernelFunc>
		struct NeighborSearchParamTmp
		{
			NeighborhoodArray &_currentPNeighborhood;

			const VectOrIntrinsType _currentPPos;
			const Storm::Vector3* _otherPPos;

			const float _kernelLength;
			const float _kernelLengthSquared;

			VectOrIntrinsType _xij;

			float _normSquared;

			const RawKernelFunc &_rawKernelFunc;
			const GradKernelFunc &_gradKernelFunc;
		};
	}

#if !STORM_USE_INTRINSICS
	template<class SearchParam>
	__forceinline bool isNeighborhood(SearchParam &inOutParam)
	{
		const Storm::Vector3 &toCheckPPos = *inOutParam._otherPPos;

		float &outX = inOutParam._xij.x();
		outX = inOutParam._currentPPos.x() - toCheckPPos.x();
		const float xDiffSquared = outX * outX;
		if (xDiffSquared < kernelLengthSquared)
		{
			float &outY = inOutParam._xij.y();
			outY = inOutParam._currentPPos.y() - toCheckPPos.y();
			const float yDiffSquared = outY * outY;
			if (yDiffSquared < kernelLengthSquared)
			{
				float &outZ = inOutParam._xij.z();
				outZ = inOutParam._currentPPos.z() - toCheckPPos.z();
				inOutParam._normSquared = xDiffSquared + yDiffSquared + outZ * outZ;

				return inOutParam._normSquared > 0.0000000000001f && inOutParam._normSquared < inOutParam._kernelLengthSquared;
			}
		}

		return false;
	}
#else
	template<class SearchParam>
	__forceinline bool isNeighborhood(SearchParam &inOutParam)
	{
		enum : int
		{
			// Masks are for those component, in this order : wzyx

			broadcastMask = 0b0001,
			conditionMask = 0b0111,

			dotProductMask = conditionMask << 4 | broadcastMask
		};

		const Storm::Vector3 &toCheckPPos = *inOutParam._otherPPos;
		inOutParam._xij = _mm_sub_ps(inOutParam._currentPPos, STORM_INTRINSICS_LOAD_PS_FROM_VECT3(toCheckPPos));

		inOutParam._normSquared = _mm_dp_ps(inOutParam._xij, inOutParam._xij, dotProductMask).m128_f32[0];
		return inOutParam._normSquared > 0.0000000000001f && inOutParam._normSquared < inOutParam._kernelLengthSquared;
	}
#endif

	template<class SearchParam>
	__forceinline void addIfNeighbor(SearchParam &param, Storm::ParticleSystem*const particleSystem, const Storm::NeighborParticleReferral &particleReferral)
	{
		if (Storm::isNeighborhood(param))
		{
#if STORM_USE_INTRINSICS
			Storm::NeighborParticleInfo &addedNeighbor = param._currentPNeighborhood.emplace_back(particleSystem, particleReferral._particleIndex, param._xij.m128_f32[0], param._xij.m128_f32[1], param._xij.m128_f32[2], param._normSquared);
#else
			Storm::NeighborParticleInfo &addedNeighbor = param._currentPNeighborhood.emplace_back(particleSystem, particleReferral._particleIndex, param._xij, param._normSquared, isFluid);
#endif

			addedNeighbor._Wij = param._rawKernelFunc(param._kernelLength, addedNeighbor._xijNorm);
			addedNeighbor._gradWij = param._gradKernelFunc(param._kernelLength, addedNeighbor._xij, addedNeighbor._xijNorm);
		}
	}

	template<bool containingBundleIsSameTypeThanThisSystemP, class NeighborhoodArray, std::size_t outLinkedNeighborBundleSize, class RawKernelFunc, class GradKernelFunc>
	void searchForNeighborhood(Storm::ParticleSystem* thisParticleSystem, const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLength, const float kernelLengthSquared, const unsigned int currentSystemId, NeighborhoodArray &currentPNeighborhood, const std::size_t particleIndex, const Storm::Vector3 &currentPPosition, const std::vector<Storm::NeighborParticleReferral> &containingBundleReferrals, const std::vector<Storm::NeighborParticleReferral>*(&outLinkedNeighborBundle)[outLinkedNeighborBundleSize], const RawKernelFunc &rawKernelFunc, const GradKernelFunc &gradKernelFunc)
	{
#if STORM_USE_INTRINSICS
		details::NeighborSearchParamTmp<__m128, NeighborhoodArray, RawKernelFunc, GradKernelFunc> param {
			._currentPNeighborhood = currentPNeighborhood,
			._currentPPos = STORM_INTRINSICS_LOAD_PS_FROM_VECT3(currentPPosition),
			._kernelLength = kernelLength,
			._kernelLengthSquared = kernelLengthSquared,
			._rawKernelFunc = rawKernelFunc,
			._gradKernelFunc = gradKernelFunc,
		};

#else
		details::NeighborSearchParamTmp<Storm::Vector3, NeighborhoodArray, RawKernelFunc, GradKernelFunc> param {
			._currentPNeighborhood = currentPNeighborhood,
			._currentPPos = currentPPosition,
			._kernelLength = kernelLength,
			._kernelLengthSquared = kernelLengthSquared,
			._rawKernelFunc = rawKernelFunc,
			._gradKernelFunc = gradKernelFunc,
		};
#endif

		Storm::ParticleSystem* otherPSystem = nullptr;
		unsigned int lastOtherPSystemCachedId = currentSystemId;

		if constexpr (containingBundleIsSameTypeThanThisSystemP)
		{
			const std::vector<Storm::Vector3> &thisSystemAllPPosition = thisParticleSystem->getPositions();

			std::size_t iter = 0;
			const std::size_t containingBundleRefCount = containingBundleReferrals.size();

			for (; iter < containingBundleRefCount; ++iter)
			{
				const Storm::NeighborParticleReferral &particleReferral = containingBundleReferrals[iter];

				if (particleReferral._systemId == currentSystemId)
				{
					if (particleReferral._particleIndex != particleIndex)
					{
						param._otherPPos = &thisSystemAllPPosition[particleReferral._particleIndex];

						Storm::addIfNeighbor(param, thisParticleSystem, particleReferral);
					}
					else
					{
						++iter; //Skip it... Current particle shouldn't be part of its own neighborhood
						break;
					}
				}
				else
				{
					if (lastOtherPSystemCachedId != particleReferral._systemId)
					{
						lastOtherPSystemCachedId = particleReferral._systemId;
						otherPSystem = allParticleSystems.find(particleReferral._systemId)->second.get();
					}

					param._otherPPos = &otherPSystem->getPositions()[particleReferral._particleIndex];

					Storm::addIfNeighbor(param, otherPSystem, particleReferral);
				}
			}

			for (; iter < containingBundleRefCount; ++iter)
			{
				const Storm::NeighborParticleReferral &particleReferral = containingBundleReferrals[iter];

				if (particleReferral._systemId == currentSystemId)
				{
					param._otherPPos = &thisSystemAllPPosition[particleReferral._particleIndex];

					Storm::addIfNeighbor(param, thisParticleSystem, particleReferral);
				}
				else
				{
					if (lastOtherPSystemCachedId != particleReferral._systemId)
					{
						lastOtherPSystemCachedId = particleReferral._systemId;
						otherPSystem = allParticleSystems.find(particleReferral._systemId)->second.get();
					}

					param._otherPPos = &otherPSystem->getPositions()[particleReferral._particleIndex];

					Storm::addIfNeighbor(param, otherPSystem, particleReferral);
				}
			}

			for (const std::vector<Storm::NeighborParticleReferral>** linkedNeighborReferralsIter = outLinkedNeighborBundle; *linkedNeighborReferralsIter != nullptr; ++linkedNeighborReferralsIter)
			{
				for (const Storm::NeighborParticleReferral &particleReferral : **linkedNeighborReferralsIter)
				{
					if (particleReferral._systemId == currentSystemId)
					{
						param._otherPPos = &thisSystemAllPPosition[particleReferral._particleIndex];

						Storm::addIfNeighbor(param, thisParticleSystem, particleReferral);
					}
					else
					{
						if (lastOtherPSystemCachedId != particleReferral._systemId)
						{
							lastOtherPSystemCachedId = particleReferral._systemId;
							otherPSystem = allParticleSystems.find(particleReferral._systemId)->second.get();
						}

						param._otherPPos = &otherPSystem->getPositions()[particleReferral._particleIndex];

						Storm::addIfNeighbor(param, otherPSystem, particleReferral);
					}
				}
			}
		}
		else
		{
			for (const Storm::NeighborParticleReferral &particleReferral : containingBundleReferrals)
			{
				if (lastOtherPSystemCachedId != particleReferral._systemId)
				{
					lastOtherPSystemCachedId = particleReferral._systemId;
					otherPSystem = allParticleSystems.find(particleReferral._systemId)->second.get();
				}

				param._otherPPos = &otherPSystem->getPositions()[particleReferral._particleIndex];

				Storm::addIfNeighbor(param, otherPSystem, particleReferral);
			}

			for (const std::vector<Storm::NeighborParticleReferral>** linkedNeighborReferralsIter = outLinkedNeighborBundle; *linkedNeighborReferralsIter != nullptr; ++linkedNeighborReferralsIter)
			{
				for (const Storm::NeighborParticleReferral &particleReferral : **linkedNeighborReferralsIter)
				{
					if (lastOtherPSystemCachedId != particleReferral._systemId)
					{
						lastOtherPSystemCachedId = particleReferral._systemId;
						otherPSystem = allParticleSystems.find(particleReferral._systemId)->second.get();
					}

					param._otherPPos = &otherPSystem->getPositions()[particleReferral._particleIndex];

					Storm::addIfNeighbor(param, otherPSystem, particleReferral);
				}
			}
		}
	}
}
