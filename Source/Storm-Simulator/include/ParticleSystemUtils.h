#pragma once


namespace Storm
{
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

	template<bool isFluid, bool containingBundleIsSameTypeThanThisSystemP, class NeighborhoodArray, std::size_t outLinkedNeighborBundleSize>
	void searchForNeighborhood(Storm::ParticleSystem* thisParticleSystem, const std::map<unsigned int, std::unique_ptr<Storm::ParticleSystem>> &allParticleSystems, const float kernelLengthSquared, const unsigned int currentSystemId, NeighborhoodArray &currentPNeighborhood, const std::size_t particleIndex, const Storm::Vector3 &currentPPosition, const std::vector<Storm::NeighborParticleReferral> &containingBundleReferrals, const std::vector<Storm::NeighborParticleReferral>*(&outLinkedNeighborBundle)[outLinkedNeighborBundleSize])
	{
		Storm::Vector3 posDiff;
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

					if (Storm::isNeighborhood(currentPPosition, thisSystemAllPPosition[particleReferral._particleIndex], kernelLengthSquared, posDiff, normSquared))
					{
						currentPNeighborhood.emplace_back(thisParticleSystem, particleReferral._particleIndex, posDiff, normSquared, isFluid);
					}
				}
				else
				{
					if (lastOtherPSystemCachedId != particleReferral._systemId)
					{
						lastOtherPSystemCachedId = particleReferral._systemId;
						otherPSystem = allParticleSystems.find(particleReferral._systemId)->second.get();
					}

					const Storm::Vector3 &otherParticlePos = otherPSystem->getPositions()[particleReferral._particleIndex];
					if (Storm::isNeighborhood(currentPPosition, otherParticlePos, kernelLengthSquared, posDiff, normSquared))
					{
						currentPNeighborhood.emplace_back(otherPSystem, particleReferral._particleIndex, posDiff, normSquared, isFluid);
					}
				}
			}

			for (; iter < containingBundleRefCount; ++iter)
			{
				const Storm::NeighborParticleReferral &particleReferral = containingBundleReferrals[iter];

				if (particleReferral._systemId == currentSystemId)
				{
					if (Storm::isNeighborhood(currentPPosition, thisSystemAllPPosition[particleReferral._particleIndex], kernelLengthSquared, posDiff, normSquared))
					{
						currentPNeighborhood.emplace_back(thisParticleSystem, particleReferral._particleIndex, posDiff, normSquared, isFluid);
					}
				}
				else
				{
					if (lastOtherPSystemCachedId != particleReferral._systemId)
					{
						lastOtherPSystemCachedId = particleReferral._systemId;
						otherPSystem = allParticleSystems.find(particleReferral._systemId)->second.get();
					}

					const Storm::Vector3 &otherParticlePos = otherPSystem->getPositions()[particleReferral._particleIndex];
					if (Storm::isNeighborhood(currentPPosition, otherParticlePos, kernelLengthSquared, posDiff, normSquared))
					{
						currentPNeighborhood.emplace_back(otherPSystem, particleReferral._particleIndex, posDiff, normSquared, isFluid);
					}
				}
			}

			for (const std::vector<Storm::NeighborParticleReferral>** linkedNeighborReferralsIter = outLinkedNeighborBundle; *linkedNeighborReferralsIter != nullptr; ++linkedNeighborReferralsIter)
			{
				for (const Storm::NeighborParticleReferral &particleReferral : **linkedNeighborReferralsIter)
				{
					if (particleReferral._systemId == currentSystemId)
					{
						if (Storm::isNeighborhood(currentPPosition, thisSystemAllPPosition[particleReferral._particleIndex], kernelLengthSquared, posDiff, normSquared))
						{
							currentPNeighborhood.emplace_back(thisParticleSystem, particleReferral._particleIndex, posDiff, normSquared, isFluid);
						}
					}
					else
					{
						if (lastOtherPSystemCachedId != particleReferral._systemId)
						{
							lastOtherPSystemCachedId = particleReferral._systemId;
							otherPSystem = allParticleSystems.find(particleReferral._systemId)->second.get();
						}

						const Storm::Vector3 &otherParticlePos = otherPSystem->getPositions()[particleReferral._particleIndex];
						if (Storm::isNeighborhood(currentPPosition, otherParticlePos, kernelLengthSquared, posDiff, normSquared))
						{
							currentPNeighborhood.emplace_back(otherPSystem, particleReferral._particleIndex, posDiff, normSquared, isFluid);
						}
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

				const Storm::Vector3 &otherParticlePos = otherPSystem->getPositions()[particleReferral._particleIndex];
				if (Storm::isNeighborhood(currentPPosition, otherParticlePos, kernelLengthSquared, posDiff, normSquared))
				{
					currentPNeighborhood.emplace_back(otherPSystem, particleReferral._particleIndex, posDiff, normSquared, isFluid);
				}
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

					const Storm::Vector3 &otherParticlePos = otherPSystem->getPositions()[particleReferral._particleIndex];
					if (Storm::isNeighborhood(currentPPosition, otherParticlePos, kernelLengthSquared, posDiff, normSquared))
					{
						currentPNeighborhood.emplace_back(otherPSystem, particleReferral._particleIndex, posDiff, normSquared, isFluid);
					}
				}
			}
		}
	}
}
