#pragma once

#include "NonInstanciable.h"
#include "SIMDUsageMode.h"


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



		template<SIMDUsageMode mode>
		static __forceinline void copyMemory_V2(const uint8_t*const srcPtr, uint8_t*const dstPtr, const std::size_t sizeInBytes)
		{
			if constexpr (mode == Storm::SIMDUsageMode::AVX512)
			{
				if (sizeInBytes >= sizeof(__m512i))
				{
					__m512i increment = _mm512_set1_epi64(sizeof(__m512i));
					__m512i storageVar = _mm512_set4_epi64(0, sizeof(__m512i), (uint64_t)dstPtr, (uint64_t)srcPtr);

					do
					{
						_mm512_storeu_epi32(reinterpret_cast<void*const>(storageVar.m512i_u64[1]), _mm512_loadu_epi32(reinterpret_cast<const void*>(storageVar.m512i_u64[0])));
						storageVar = _mm512_add_epi64(storageVar, increment);
					} while (storageVar.m512i_u64[2] < sizeInBytes);

					storageVar = _mm512_sub_epi64(storageVar, _mm512_set1_epi64(storageVar.m512i_u64[2] - sizeInBytes));
					_mm512_storeu_epi32(reinterpret_cast<void*const>(storageVar.m512i_u64[1]), _mm512_loadu_epi32(reinterpret_cast<const void*>(storageVar.m512i_u64[0])));
					return;
				}
			}
			else if constexpr (mode == Storm::SIMDUsageMode::SSE)
			{
				if (sizeInBytes >= sizeof(__m128i))
				{
					uint64_t iter = sizeof(__m128i);
					__m128i increment = _mm_set1_epi64x(sizeof(__m128i));
					__m128i storageVar = _mm_set_epi64x((uint64_t)srcPtr, (uint64_t)dstPtr);

					do
					{
						*reinterpret_cast<__m128i*>(storageVar.m128i_u64[0]) = *reinterpret_cast<__m128i*>(storageVar.m128i_u64[1]);
						storageVar = _mm_add_epi64(storageVar, increment);

						iter += sizeof(__m128i);
					} while (iter < sizeInBytes);


					storageVar = _mm_sub_epi64(storageVar, _mm_set1_epi64x(iter - sizeInBytes));
					*reinterpret_cast<__m128i*>(storageVar.m128i_u64[0]) = *reinterpret_cast<__m128i*>(storageVar.m128i_u64[1]);

					return;
				}
			}

			// Fallback
			::memcpy(dstPtrAsInt, srcPtrAsInt, sizeInBytes);
		}

		template<SIMDUsageMode mode, class Type>
		static __forceinline void copyMemory_V2(const std::vector<Type> &src, std::vector<Type> &dst)
		{
			assert(src.size() == dst.size() && "Size mismatch!");
			Storm::FastOperation::copyMemory_V2<mode>(src.data(), dst.data(), dst.size() * sizeof(Type));
		}

	public:
		static bool canUseSIMD();
		static bool canUseAVX512();
	};
}
