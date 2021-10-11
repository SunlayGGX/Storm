#pragma once

#include "ParticleSystemContainer.h"

#include "OutReflectedModality.h"


namespace Storm
{
	namespace details
	{
		template<class VectOrIntrinsType>
		struct NeighborSearchParamTmp
		{
			const VectOrIntrinsType _currentPPos;
			VectOrIntrinsType _otherPPos;

			VectOrIntrinsType _xij;

			float _normSquared;

			VectOrIntrinsType _domainReflectionDimension;
		};
	}

	template<class NeighborhoodArrayType, class RawKernelFuncType, class GradKernelFuncType, std::size_t outLinkedNeighborBundleSize>
	struct NeighborSearchInParam
	{
	public:
		using NeighborhoodArray = NeighborhoodArrayType;
		using RawKernelFunc = RawKernelFuncType;
		using GradKernelFunc = GradKernelFuncType;

	public:
		Storm::ParticleSystem*const _thisParticleSystem;
		const Storm::ParticleSystemContainer &_allParticleSystems;
		const float _kernelLength;
		const float _kernelLengthSquared;
		const unsigned int _currentSystemId;
		NeighborhoodArray &_currentPNeighborhood;
		const std::size_t _particleIndex;
		const Storm::Vector3 &_currentPPosition;
		const std::vector<Storm::NeighborParticleReferral>* &_containingBundleReferrals;
		const std::vector<Storm::NeighborParticleReferral>*(&_outLinkedNeighborBundle)[outLinkedNeighborBundleSize];
		const RawKernelFunc &_rawKernelFunc;
		const GradKernelFunc &_gradKernelFunc;
		const Storm::Vector3 &_domainDimension;

		bool _isFluid;
		const Storm::OutReflectedModality* _reflectedModality;
	};

	namespace
	{
		using OutReflectedModalityEnumUnderlyingNative = Storm::ByteValueType<Storm::BitsCount<Storm::OutReflectedModalityEnum>::k_value>::Type;
	}

#define STORM_PROCESS_RETURN_DIMENSION_FROM_FLAG(FlagPlus, FlagMinus, Coord) \
	(static_cast<OutReflectedModalityEnumUnderlyingNative>(reflectedModality) & static_cast<OutReflectedModalityEnumUnderlyingNative>(Storm::OutReflectedModalityEnum::FlagPlus)) ? domainDimension.Coord() : ((static_cast<OutReflectedModalityEnumUnderlyingNative>(reflectedModality) & static_cast<OutReflectedModalityEnumUnderlyingNative>(Storm::OutReflectedModalityEnum::FlagMinus)) ? -domainDimension.Coord() : 0.f)

#if !STORM_USE_INTRINSICS
	template<class SearchParam>
	__forceinline bool isNeighborhood(const NeighborSearchInParamType &inParam, SearchParam &inOutParam)
	{
		const Storm::Vector3 &toCheckPPos = inOutParam._otherPPos;

		float &outX = inOutParam._xij.x();
		outX = inOutParam._currentPPos.x() - toCheckPPos.x();
		const float xDiffSquared = outX * outX;
		if (xDiffSquared < inParam._kernelLengthSquared)
		{
			float &outY = inOutParam._xij.y();
			outY = inOutParam._currentPPos.y() - toCheckPPos.y();
			const float yDiffSquared = outY * outY;
			if (yDiffSquared < inParam._kernelLengthSquared)
			{
				float &outZ = inOutParam._xij.z();
				outZ = inOutParam._currentPPos.z() - toCheckPPos.z();
				inOutParam._normSquared = xDiffSquared + yDiffSquared + outZ * outZ;

				return inOutParam._normSquared > 0.0000000000001f && inOutParam._normSquared < inParam._kernelLengthSquared;
			}
		}

		return false;
	}

	template<bool considerInfiniteDomain, class SearchParam>
	__forceinline void retrieveNeighborPosition(SearchParam &inOutParam, const std::vector<Storm::Vector3> &positions, const Storm::NeighborParticleReferral &particleReferral)
	{
		const Storm::Vector3 &position = positions[particleReferral._particleIndex];
		inOutParam._otherPPos = position;

		if constexpr (considerInfiniteDomain)
		{
			inOutParam._otherPPos += inOutParam._domainReflectionDimension;
		}
	}

	__forceinline Storm::Vector3 retrieveDomainReflectionDimension(const Storm::Vector3 &domainDimension, const Storm::OutReflectedModalityEnum reflectedModality)
	{
		return Storm::Vector3{
			STORM_PROCESS_RETURN_DIMENSION_FROM_FLAG(XReflectedToTheLeft, XReflectedToTheRight, x),
			STORM_PROCESS_RETURN_DIMENSION_FROM_FLAG(YReflectedToTheBottom, YReflectedToTheTop, y),
			STORM_PROCESS_RETURN_DIMENSION_FROM_FLAG(ZReflectedToTheFront, ZReflectedToTheBack, z)
		};
	}

#else
	template<class SearchParam, class NeighborSearchInParamType>
	__forceinline bool isNeighborhood(const NeighborSearchInParamType &inParam, SearchParam &inOutParam)
	{
		enum : int
		{
			// Masks are for those component, in this order : wzyx

			broadcastMask = 0b0001,
			conditionMask = 0b0111,

			dotProductMask = conditionMask << 4 | broadcastMask
		};

		inOutParam._xij = _mm_sub_ps(inOutParam._currentPPos, inOutParam._otherPPos);

		inOutParam._normSquared = _mm_dp_ps(inOutParam._xij, inOutParam._xij, dotProductMask).m128_f32[0];
		return inOutParam._normSquared > 0.0000000000001f && inOutParam._normSquared < inParam._kernelLengthSquared;
	}

	template<bool considerInfiniteDomain, class SearchParam>
	__forceinline void retrieveNeighborPosition(SearchParam &inOutParam, const std::vector<Storm::Vector3> &positions, const Storm::NeighborParticleReferral &particleReferral)
	{
		const Storm::Vector3 &position = positions[particleReferral._particleIndex];
		inOutParam._otherPPos = STORM_INTRINSICS_LOAD_PS_FROM_VECT3(position);

		if constexpr (considerInfiniteDomain)
		{
			inOutParam._otherPPos = _mm_add_ps(inOutParam._otherPPos, inOutParam._domainReflectionDimension);
		}
	}

	__forceinline __m128 retrieveDomainReflectionDimension(const Storm::Vector3 &domainDimension, const Storm::OutReflectedModalityEnum reflectedModality)
	{
		return _mm_setr_ps(
			STORM_PROCESS_RETURN_DIMENSION_FROM_FLAG(XReflectedToTheLeft, XReflectedToTheRight, x),
			STORM_PROCESS_RETURN_DIMENSION_FROM_FLAG(YReflectedToTheBottom, YReflectedToTheTop, y),
			STORM_PROCESS_RETURN_DIMENSION_FROM_FLAG(ZReflectedToTheFront, ZReflectedToTheBack, z),
			0.f
		);
	}

#endif

#undef STORM_PROCESS_RETURN_DIMENSION_FROM_FLAG

	template<class SearchParam, class NeighborSearchInParamType>
	__forceinline void addIfNeighbor(const NeighborSearchInParamType &inParam, SearchParam &param, Storm::ParticleSystem*const particleSystem, const Storm::NeighborParticleReferral &particleReferral)
	{
		if (Storm::isNeighborhood(inParam, param))
		{
#if STORM_USE_INTRINSICS
			Storm::NeighborParticleInfo &addedNeighbor = inParam._currentPNeighborhood.emplace_back(particleSystem, particleReferral._particleIndex, param._xij.m128_f32[0], param._xij.m128_f32[1], param._xij.m128_f32[2], param._normSquared, inParam._isFluid);
#else
			Storm::NeighborParticleInfo &addedNeighbor = inParam._currentPNeighborhood.emplace_back(particleSystem, particleReferral._particleIndex, param._xij, param._normSquared, inParam._isFluid);
#endif

			addedNeighbor._Wij = inParam._rawKernelFunc(inParam._kernelLength, addedNeighbor._xijNorm);
			addedNeighbor._gradWij = inParam._gradKernelFunc(inParam._kernelLength, addedNeighbor._xij, addedNeighbor._xijNorm);
		}
	}

	template<bool containingBundleIsSameTypeThanThisSystemP, bool considerInfiniteDomain, class NeighborSearchParamType>
	void searchForNeighborhood(const NeighborSearchParamType &inParam)
	{
#if STORM_USE_INTRINSICS
		details::NeighborSearchParamTmp<__m128> param {
			._currentPPos = STORM_INTRINSICS_LOAD_PS_FROM_VECT3(inParam._currentPPosition)
		};

#else
		details::NeighborSearchParamTmp<Storm::Vector3> param {
			._currentPPos = inParam._currentPPosition
		};
#endif

		Storm::ParticleSystem* otherPSystem = nullptr;
		unsigned int lastOtherPSystemCachedId = inParam._currentSystemId;

		if constexpr (containingBundleIsSameTypeThanThisSystemP)
		{
			const std::vector<Storm::Vector3> &thisSystemAllPPosition = inParam._thisParticleSystem->getPositions();

			std::size_t iter = 0;
			const std::size_t containingBundleRefCount = inParam._containingBundleReferrals->size();

			for (; iter < containingBundleRefCount; ++iter)
			{
				const Storm::NeighborParticleReferral &particleReferral = (*inParam._containingBundleReferrals)[iter];

				if (particleReferral._systemId == inParam._currentSystemId)
				{
					if (particleReferral._particleIndex != inParam._particleIndex)
					{
						// We're on the containing bundle, there is no reflection on the current containing bundle
						// in other words, particle from the same bundle cannot be at the other side of the domain,
						// otherwise they wouldn't be in the same bundle. Reflection are always for the neighbor bundles.
						Storm::retrieveNeighborPosition<false>(param, thisSystemAllPPosition, particleReferral);

						Storm::addIfNeighbor(inParam, param, inParam._thisParticleSystem, particleReferral);
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
						otherPSystem = inParam._allParticleSystems.find(particleReferral._systemId)->second.get();
					}

					// We're on the containing bundle, there is no reflection on the current containing bundle
					// in other words, particle from the same bundle cannot be at the other side of the domain,
					// otherwise they wouldn't be in the same bundle. Reflection are always for the neighbor bundles.
					Storm::retrieveNeighborPosition<false>(param, otherPSystem->getPositions(), particleReferral);

					Storm::addIfNeighbor(inParam, param, otherPSystem, particleReferral);
				}
			}

			for (; iter < containingBundleRefCount; ++iter)
			{
				const Storm::NeighborParticleReferral &particleReferral = (*inParam._containingBundleReferrals)[iter];

				if (particleReferral._systemId == inParam._currentSystemId)
				{
					// We're on the containing bundle, there is no reflection on the current containing bundle
					// in other words, particle from the same bundle cannot be at the other side of the domain,
					// otherwise they wouldn't be in the same bundle. Reflection are always for the neighbor bundles.
					Storm::retrieveNeighborPosition<false>(param, thisSystemAllPPosition, particleReferral);

					Storm::addIfNeighbor(inParam, param, inParam._thisParticleSystem, particleReferral);
				}
				else
				{
					if (lastOtherPSystemCachedId != particleReferral._systemId)
					{
						lastOtherPSystemCachedId = particleReferral._systemId;
						otherPSystem = inParam._allParticleSystems.find(particleReferral._systemId)->second.get();
					}

					// We're on the containing bundle, there is no reflection on the current containing bundle
					// in other words, particle from the same bundle cannot be at the other side of the domain,
					// otherwise they wouldn't be in the same bundle. Reflection are always for the neighbor bundles.
					Storm::retrieveNeighborPosition<false>(param, otherPSystem->getPositions(), particleReferral);

					Storm::addIfNeighbor(inParam, param, otherPSystem, particleReferral);
				}
			}

			for (const std::vector<Storm::NeighborParticleReferral>** linkedNeighborReferralsIter = inParam._outLinkedNeighborBundle; *linkedNeighborReferralsIter != nullptr; ++linkedNeighborReferralsIter)
			{
				bool noReflect;
				if constexpr (considerInfiniteDomain)
				{
					const Storm::OutReflectedModalityEnum reflectedModality = inParam._reflectedModality->_modalityPerBundle[linkedNeighborReferralsIter - inParam._outLinkedNeighborBundle];
					if (reflectedModality == Storm::OutReflectedModalityEnum::None)
					{
						noReflect = true;
					}
					else
					{
						noReflect = false;
						param._domainReflectionDimension = retrieveDomainReflectionDimension(inParam._domainDimension, reflectedModality);
					}
				}
				else
				{
					noReflect = true;
				}

				if (noReflect)
				{
					for (const Storm::NeighborParticleReferral &particleReferral : **linkedNeighborReferralsIter)
					{
						if (particleReferral._systemId == inParam._currentSystemId)
						{
							Storm::retrieveNeighborPosition<false>(param, thisSystemAllPPosition, particleReferral);

							Storm::addIfNeighbor(inParam, param, inParam._thisParticleSystem, particleReferral);
						}
						else
						{
							if (lastOtherPSystemCachedId != particleReferral._systemId)
							{
								lastOtherPSystemCachedId = particleReferral._systemId;
								otherPSystem = inParam._allParticleSystems.find(particleReferral._systemId)->second.get();
							}

							Storm::retrieveNeighborPosition<false>(param, otherPSystem->getPositions(), particleReferral);

							Storm::addIfNeighbor(inParam, param, otherPSystem, particleReferral);
						}
					}
				}
				else
				{
					for (const Storm::NeighborParticleReferral &particleReferral : **linkedNeighborReferralsIter)
					{
						if (particleReferral._systemId == inParam._currentSystemId)
						{
							Storm::retrieveNeighborPosition<considerInfiniteDomain>(param, thisSystemAllPPosition, particleReferral);

							Storm::addIfNeighbor(inParam, param, inParam._thisParticleSystem, particleReferral);
						}
						else
						{
							if (lastOtherPSystemCachedId != particleReferral._systemId)
							{
								lastOtherPSystemCachedId = particleReferral._systemId;
								otherPSystem = inParam._allParticleSystems.find(particleReferral._systemId)->second.get();
							}

							Storm::retrieveNeighborPosition<considerInfiniteDomain>(param, otherPSystem->getPositions(), particleReferral);

							Storm::addIfNeighbor(inParam, param, otherPSystem, particleReferral);
						}
					}
				}
			}
		}
		else
		{
			for (const Storm::NeighborParticleReferral &particleReferral : *inParam._containingBundleReferrals)
			{
				if (lastOtherPSystemCachedId != particleReferral._systemId)
				{
					lastOtherPSystemCachedId = particleReferral._systemId;
					otherPSystem = inParam._allParticleSystems.find(particleReferral._systemId)->second.get();
				}

				// We're on the containing bundle, there is no reflection on the current containing bundle
				// in other words, particle from the same bundle cannot be at the other side of the domain,
				// otherwise they wouldn't be in the same bundle. Reflection are always for the neighbor bundles.
				Storm::retrieveNeighborPosition<false>(param, otherPSystem->getPositions(), particleReferral);

				Storm::addIfNeighbor(inParam, param, otherPSystem, particleReferral);
			}

			for (const std::vector<Storm::NeighborParticleReferral>** linkedNeighborReferralsIter = inParam._outLinkedNeighborBundle; *linkedNeighborReferralsIter != nullptr; ++linkedNeighborReferralsIter)
			{
				bool noReflect;
				if constexpr (considerInfiniteDomain)
				{
					const Storm::OutReflectedModalityEnum reflectedModality = inParam._reflectedModality->_modalityPerBundle[linkedNeighborReferralsIter - inParam._outLinkedNeighborBundle];
					if (reflectedModality == Storm::OutReflectedModalityEnum::None)
					{
						noReflect = true;
					}
					else
					{
						noReflect = false;
						param._domainReflectionDimension = retrieveDomainReflectionDimension(inParam._domainDimension, reflectedModality);
					}
				}
				else
				{
					noReflect = true;
				}

				if (noReflect)
				{
					for (const Storm::NeighborParticleReferral &particleReferral : **linkedNeighborReferralsIter)
					{
						if (lastOtherPSystemCachedId != particleReferral._systemId)
						{
							lastOtherPSystemCachedId = particleReferral._systemId;
							otherPSystem = inParam._allParticleSystems.find(particleReferral._systemId)->second.get();
						}

						Storm::retrieveNeighborPosition<false>(param, otherPSystem->getPositions(), particleReferral);

						Storm::addIfNeighbor(inParam, param, otherPSystem, particleReferral);
					}
				}
				else
				{
					for (const Storm::NeighborParticleReferral &particleReferral : **linkedNeighborReferralsIter)
					{
						if (lastOtherPSystemCachedId != particleReferral._systemId)
						{
							lastOtherPSystemCachedId = particleReferral._systemId;
							otherPSystem = inParam._allParticleSystems.find(particleReferral._systemId)->second.get();
						}

						Storm::retrieveNeighborPosition<considerInfiniteDomain>(param, otherPSystem->getPositions(), particleReferral);

						Storm::addIfNeighbor(inParam, param, otherPSystem, particleReferral);
					}
				}
			}
		}
	}
}
