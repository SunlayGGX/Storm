#pragma once


namespace Storm
{
#if !STORM_USE_INTRINSICS
	__forceinline bool isNeighborhood(const Storm::Vector3 &currentPPos, const Storm::Vector3 &toCheckPPos, const float kernelLengthSquared, Storm::Vector3 &outPosDiff, float &normSquared)
	{
		float &outX = outPosDiff.x();
		outX = currentPPos.x() - toCheckPPos.x();
		const float xDiffSquared = outX * outX;
		if (xDiffSquared < kernelLengthSquared)
		{
			float &outY = outPosDiff.y();
			outY = currentPPos.y() - toCheckPPos.y();
			const float yDiffSquared = outY * outY;
			if (yDiffSquared < kernelLengthSquared)
			{
				float &outZ = outPosDiff.z();
				outZ = currentPPos.z() - toCheckPPos.z();
				normSquared = xDiffSquared + yDiffSquared + outZ * outZ;

				return normSquared > 0.000000001f && normSquared < kernelLengthSquared;
			}
		}

		return false;
	}
#else
	__forceinline bool isNeighborhood(const __m128 &currentPPos, const Storm::Vector3 &toCheckPPos, const float kernelLengthSquared, __m128 &outPosDiff, float &normSquared)
	{
		enum : int
		{
			// Masks are for those component, in this order : wzyx

			broadcastMask = 0b0001,
			conditionMask = 0b0111,

			dotProductMask = conditionMask << 4 | broadcastMask
		};

		outPosDiff = _mm_sub_ps(currentPPos, _mm_setr_ps(toCheckPPos.x(), toCheckPPos.y(), toCheckPPos.z(), 0.f));

		normSquared = _mm_dp_ps(outPosDiff, outPosDiff, dotProductMask).m128_f32[0];
		return normSquared > 0.000000001f && normSquared < kernelLengthSquared;
	}
#endif

	template<bool isFluid, class VectOrIntrinsType, class NeighborhoodArray>
	__forceinline void addIfNeighbor(NeighborhoodArray &currentPNeighborhood, const VectOrIntrinsType &currentPPos, const Storm::Vector3 &otherPPos, const float kernelLengthSquared,  Storm::ParticleSystem* particleSystem, const Storm::NeighborParticleReferral &particleReferral, VectOrIntrinsType &posDiff, float &normSquared)
	{
		if (Storm::isNeighborhood(currentPPos, otherPPos, kernelLengthSquared, posDiff, normSquared))
		{
#if STORM_USE_INTRINSICS
			currentPNeighborhood.emplace_back(particleSystem, particleReferral._particleIndex, posDiff.m128_f32[0], posDiff.m128_f32[1], posDiff.m128_f32[2], normSquared, isFluid);
#else
			currentPNeighborhood.emplace_back(particleSystem, particleReferral._particleIndex, posDiff, normSquared, isFluid);
#endif
		}
	}

	template<bool isFluid, bool containingBundleIsSameTypeThanThisSystemP, class NeighborhoodArray, std::size_t outLinkedNeighborBundleSize>
	void searchForNeighborhood(Storm::ParticleSystem* thisParticleSystem, const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &allParticleSystems, const float kernelLengthSquared, const unsigned int currentSystemId, NeighborhoodArray &currentPNeighborhood, const std::size_t particleIndex, const Storm::Vector3 &currentPPosition, const std::vector<Storm::NeighborParticleReferral> &containingBundleReferrals, const std::vector<Storm::NeighborParticleReferral>*(&outLinkedNeighborBundle)[outLinkedNeighborBundleSize])
	{
#if STORM_USE_INTRINSICS
		const __m128 currentPPosTmp = _mm_setr_ps(currentPPosition.x(), currentPPosition.y(), currentPPosition.z(), 0.f);
		__m128 posDiff;
#else
		const Storm::Vector3 &currentPPosTmp = currentPPosition;
		Storm::Vector3 posDiff;
#endif

		float normSquared;

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
					if (particleReferral._particleIndex == particleIndex)
					{
						++iter; //Skip it... Current particle shouldn't be part of its own neighborhood
						break;
					}

					Storm::addIfNeighbor<isFluid>(currentPNeighborhood, currentPPosTmp, thisSystemAllPPosition[particleReferral._particleIndex], kernelLengthSquared, thisParticleSystem, particleReferral, posDiff, normSquared);
				}
				else
				{
					if (lastOtherPSystemCachedId != particleReferral._systemId)
					{
						lastOtherPSystemCachedId = particleReferral._systemId;
						otherPSystem = allParticleSystems.find(particleReferral._systemId)->second.get();
					}

					Storm::addIfNeighbor<isFluid>(currentPNeighborhood, currentPPosTmp, otherPSystem->getPositions()[particleReferral._particleIndex], kernelLengthSquared, otherPSystem, particleReferral, posDiff, normSquared);
				}
			}

			for (; iter < containingBundleRefCount; ++iter)
			{
				const Storm::NeighborParticleReferral &particleReferral = containingBundleReferrals[iter];

				if (particleReferral._systemId == currentSystemId)
				{

					Storm::addIfNeighbor<isFluid>(currentPNeighborhood, currentPPosTmp, thisSystemAllPPosition[particleReferral._particleIndex], kernelLengthSquared, thisParticleSystem, particleReferral, posDiff, normSquared);
				}
				else
				{
					if (lastOtherPSystemCachedId != particleReferral._systemId)
					{
						lastOtherPSystemCachedId = particleReferral._systemId;
						otherPSystem = allParticleSystems.find(particleReferral._systemId)->second.get();
					}

					Storm::addIfNeighbor<isFluid>(currentPNeighborhood, currentPPosTmp, otherPSystem->getPositions()[particleReferral._particleIndex], kernelLengthSquared, otherPSystem, particleReferral, posDiff, normSquared);
				}
			}

			for (const std::vector<Storm::NeighborParticleReferral>** linkedNeighborReferralsIter = outLinkedNeighborBundle; *linkedNeighborReferralsIter != nullptr; ++linkedNeighborReferralsIter)
			{
				for (const Storm::NeighborParticleReferral &particleReferral : **linkedNeighborReferralsIter)
				{
					if (particleReferral._systemId == currentSystemId)
					{
						Storm::addIfNeighbor<isFluid>(currentPNeighborhood, currentPPosTmp, thisSystemAllPPosition[particleReferral._particleIndex], kernelLengthSquared, thisParticleSystem, particleReferral, posDiff, normSquared);
					}
					else
					{
						if (lastOtherPSystemCachedId != particleReferral._systemId)
						{
							lastOtherPSystemCachedId = particleReferral._systemId;
							otherPSystem = allParticleSystems.find(particleReferral._systemId)->second.get();
						}

						Storm::addIfNeighbor<isFluid>(currentPNeighborhood, currentPPosTmp, otherPSystem->getPositions()[particleReferral._particleIndex], kernelLengthSquared, otherPSystem, particleReferral, posDiff, normSquared);
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

				Storm::addIfNeighbor<isFluid>(currentPNeighborhood, currentPPosTmp, otherPSystem->getPositions()[particleReferral._particleIndex], kernelLengthSquared, otherPSystem, particleReferral, posDiff, normSquared);
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

					Storm::addIfNeighbor<isFluid>(currentPNeighborhood, currentPPosTmp, otherPSystem->getPositions()[particleReferral._particleIndex], kernelLengthSquared, otherPSystem, particleReferral, posDiff, normSquared);
				}
			}
		}
	}
}
