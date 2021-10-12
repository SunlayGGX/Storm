#pragma once

#include "StaticAssertionsMacros.h"


namespace Storm
{
	template<class ValueType>
	struct BitsCount
	{
		enum
		{
			k_value = sizeof(ValueType) * 8
		};
	};

	template<std::size_t k_bitCount>
	struct ByteValueType
	{
		using Type =
			std::conditional_t<(k_bitCount <= Storm::BitsCount<uint8_t>::k_value), uint8_t,
			std::conditional_t<(k_bitCount <= Storm::BitsCount<uint16_t>::k_value), uint16_t,
			std::conditional_t<(k_bitCount <= Storm::BitsCount<uint32_t>::k_value), uint32_t,
			std::conditional_t<(k_bitCount <= Storm::BitsCount<uint64_t>::k_value), uint64_t,
			void>>>>;
	};

	template<class EnumType> using EnumUnderlyingNative = Storm::ByteValueType<Storm::BitsCount<EnumType>::k_value>::template Type;

	template<bool bit, bool ... othersBits>
	struct BitField
	{
	public:
		enum
		{
			bitCount = sizeof...(othersBits) + 1
		};

	private:
		using ValueType = Storm::ByteValueType<bitCount>::template Type;

	public:
		enum : ValueType
		{
			value = (static_cast<ValueType>(BitField<bit>::value) << sizeof...(othersBits)) | Storm::BitField<othersBits...>::value
		};
	};

	template<bool bit>
	struct BitField<bit>
	{
		enum : uint8_t
		{
			value = bit ? 0x1 : 0x0
		};
	};

	template<class ValueType, ValueType val, class BitsType>
	__forceinline constexpr bool isBitEnabled(const BitsType bits)
	{
		enum 
		{
			bitCount1 = sizeof(BitsType) * 8,
			bitCount2 = sizeof(ValueType) * 8,
		};

		using NumericType1 = Storm::ByteValueType<bitCount1>::template Type;
		using NumericType2 = Storm::ByteValueType<bitCount2>::template Type;
		using NumericType = std::conditional_t<(sizeof(NumericType1) > sizeof(NumericType2)), NumericType1, NumericType2>;

		return static_cast<NumericType>(bits) & static_cast<NumericType>(val);
	}


	template<class ValueType, ValueType val, class BitsType>
	__forceinline void addBitFlag(BitsType &bits)
	{
		enum
		{
			bitCount = sizeof(BitsType) * 8
		};

		using NumericType = Storm::ByteValueType<bitCount>::template Type;

		STORM_STATIC_ASSERT(
			sizeof(NumericType) >= sizeof(Storm::ByteValueType<sizeof(ValueType) * 8>::template Type),
			"Cannot set bits with a bigger flag that the BitsType can hold."
		);

		bits = static_cast<BitsType>(static_cast<NumericType>(bits) | static_cast<NumericType>(val));
	}

#define STORM_IS_BIT_ENABLED(flag, bitValueVar) Storm::isBitEnabled<decltype(bitValueVar), bitValueVar>(flag)
#define STORM_ADD_BIT_ENABLED(flag, bitValueVar) Storm::addBitFlag<decltype(bitValueVar), bitValueVar>(flag)
}
