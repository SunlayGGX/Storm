#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class FastOperation : private Storm::NonInstanciable
	{
	public:
		template<bool useSIMD, bool useAVX512, class PtrType>
		static __forceinline void zeroMemory(PtrType*const ptr, const std::size_t size)
		{
			int32_t* ptrAsInt = reinterpret_cast<int32_t*>(ptr);
			std::size_t sizeInBytes = size * sizeof(PtrType);

			if constexpr (useSIMD)
			{
				if constexpr (useAVX512)
				{
					enum : std::size_t
					{
						k_avx512Shift = 16,
						k_avx512Stop = (k_avx512Shift - 1) * sizeof(float),
						k_avx512SizeShift = k_avx512Shift * sizeof(float),
					};
					
					if (sizeInBytes > k_avx512Stop)
					{
						const __m512i val = _mm512_set1_epi32(0);
						do
						{
							_mm512_storeu_epi32(ptrAsInt, val);
							ptrAsInt += k_avx512Shift;
							sizeInBytes -= k_avx512SizeShift;
						} while (sizeInBytes > k_avx512Stop);
					}
				}

				enum : std::size_t
				{
					k_sseShift = 4,
					k_sseStop = (k_sseShift - 1) * sizeof(float),
					k_sseSizeShift = k_sseShift * sizeof(float),
				};

				if (sizeInBytes > k_sseStop)
				{
					const __m128i val = _mm_set1_epi32(0);
					do
					{
						_mm_storeu_epi32(ptrAsInt, val);
						ptrAsInt += k_sseShift;
						sizeInBytes -= k_sseSizeShift;
					} while (sizeInBytes > k_sseStop);
				}
			}

			::memset(ptrAsInt, 0, sizeInBytes);
		}

		template<bool useSIMD, bool useAVX512, class Type>
		static __forceinline void zeroMemory(std::vector<Type> &inOutContainer)
		{
			Storm::FastOperation::zeroMemory<useSIMD, useAVX512>(inOutContainer.data(), inOutContainer.size());
		}


		template<bool useSIMD, bool useAVX512, class PtrType>
		static __forceinline void copyMemory(const PtrType*const srcPtr, PtrType*const dstPtr, const std::size_t size)
		{
			const int32_t* srcPtrAsInt = reinterpret_cast<const int32_t*>(srcPtr);
			int32_t* dstPtrAsInt = reinterpret_cast<int32_t*>(dstPtr);
			std::size_t sizeInBytes = size * sizeof(PtrType);

			if constexpr (useSIMD)
			{
				if constexpr (useAVX512)
				{
					enum : std::size_t
					{
						k_avx512Shift = 16,
						k_avx512Stop = (k_avx512Shift - 1) * sizeof(float),
						k_avx512SizeShift = k_avx512Shift * sizeof(float),
					};

					if (sizeInBytes > k_avx512Stop)
					{
						do
						{
							_mm512_storeu_epi32(dstPtrAsInt, _mm512_loadu_epi32(srcPtrAsInt));
							dstPtrAsInt += k_avx512Shift;
							srcPtrAsInt += k_avx512Shift;
							sizeInBytes -= k_avx512SizeShift;
						} while (sizeInBytes > k_avx512Stop);
					}
				}

				enum : std::size_t
				{
					k_sseShift = 4,
					k_sseStop = (k_sseShift - 1) * sizeof(float),
					k_sseSizeShift = k_sseShift * sizeof(float),
				};

				if (sizeInBytes > k_sseStop)
				{
					const __m128i val = _mm_set1_epi32(0);
					do
					{
						_mm_storeu_epi32(dstPtrAsInt, _mm_loadu_epi32(srcPtrAsInt));
						dstPtrAsInt += k_sseShift;
						srcPtrAsInt += k_sseShift;
						sizeInBytes -= k_sseSizeShift;
					} while (sizeInBytes > k_sseStop);
				}
			}

			::memcpy(dstPtrAsInt, srcPtrAsInt, sizeInBytes);
		}

		template<bool useSIMD, bool useAVX512, class Type>
		static __forceinline void copyMemory(const std::vector<Type> &src, std::vector<Type> &dst)
		{
			assert(src.size() == dst.size() && "Size mismatch!");
			Storm::FastOperation::copyMemory<useSIMD, useAVX512>(src.data(), dst.data(), dst.size());
		}

	public:
		static bool canUseSIMD();
		static bool canUseAVX512();
	};
}
