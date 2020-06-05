#pragma once

#include "CRTPHierarchy.h"


namespace Storm
{
	namespace details
	{
		template<class ValueType>
		class Destroyer
		{
		private:
			template<class ValueType>
			static auto destroyPointer(ValueType* ptr, int) -> decltype(ptr->release())
			{
				if (ptr != nullptr)
				{
					ptr->release();
				}
			}

			template<class ValueType>
			static auto destroyPointer(ValueType* ptr, int) -> decltype(ptr->Release())
			{
				if (ptr != nullptr)
				{
					ptr->Release();
				}
			}

		public:
			template<class ValueType>
			auto operator()(ValueType* ptr) -> decltype(Destroyer::destroyPointer(ptr, 0), void())
			{
				Destroyer::destroyPointer(ptr, 0);
			}
		};

		template<class ValueType>
		struct DestroyerTraits
		{
		private:
			template<class ValueType>
			constexpr static auto hasCustomDestroyerImpl(int) -> decltype(std::declval<Storm::details::Destroyer<ValueType>>()((ValueType*)(nullptr)), bool())
			{
				return true;
			}

			template<class ValueType>
			constexpr static bool hasCustomDestroyerImpl(void*)
			{
				return false;
			}

		public:
			enum
			{
				hasCustomDestroyer = hasCustomDestroyerImpl<ValueType>(0)
			};
		};
	};



	template<class ValueType, bool customDestroyer = static_cast<bool>(Storm::details::DestroyerTraits<ValueType>::hasCustomDestroyer)>
	class UniquePointer :
		private std::unique_ptr<ValueType, Storm::details::Destroyer<ValueType>>,
		private Storm::FullEquatable<UniquePointer<ValueType, customDestroyer>, std::nullptr_t>
	{
	public:
		using UnderlyingPtrType = std::unique_ptr<ValueType, Storm::details::Destroyer<ValueType>>;
		using UnderlyingPtrType::UnderlyingPtrType;
		using UnderlyingPtrType::get;
		using UnderlyingPtrType::release;
		using UnderlyingPtrType::reset;
		using UnderlyingPtrType::get_deleter;
		using UnderlyingPtrType::swap;
		using UnderlyingPtrType::operator ->;
		using UnderlyingPtrType::operator *;
		using UnderlyingPtrType::operator bool;
		using UnderlyingPtrType::operator =;

		using typename UnderlyingPtrType::pointer;
		using typename UnderlyingPtrType::element_type;

	public:
		UniquePointer(ValueType* ptr = nullptr) :
			UnderlyingPtrType{ ptr, Storm::details::Destroyer<ValueType>{} }
		{}

	public:
		operator UnderlyingPtrType&() { return *this; }
		operator const UnderlyingPtrType&() const { return *this; }

		bool operator==(const std::nullptr_t ptr) const
		{
			return static_cast<const UnderlyingPtrType &>(*this) == ptr;
		}
	};

	template<class ValueType>
	class UniquePointer<ValueType, false> :
		private std::unique_ptr<ValueType>,
		private Storm::FullEquatable<UniquePointer<ValueType, false>, std::nullptr_t>
	{
	public:
		using UnderlyingPtrType = std::unique_ptr<ValueType>;
		using UnderlyingPtrType::UnderlyingPtrType;
		using UnderlyingPtrType::get;
		using UnderlyingPtrType::release;
		using UnderlyingPtrType::reset;
		using UnderlyingPtrType::get_deleter;
		using UnderlyingPtrType::swap;
		using UnderlyingPtrType::operator ->;
		using UnderlyingPtrType::operator *;
		using UnderlyingPtrType::operator bool;
		using UnderlyingPtrType::operator =;

		using typename UnderlyingPtrType::pointer;
		using typename UnderlyingPtrType::element_type;

	public:
		operator UnderlyingPtrType&() { return *this; }
		operator const UnderlyingPtrType&() const { return *this; }

	public:
		UniquePointer(ValueType* ptr = nullptr) : UnderlyingPtrType{ ptr } {}

		bool operator==(const std::nullptr_t ptr) const
		{
			return static_cast<const UnderlyingPtrType &>(*this) == ptr;
		}
	};
}

namespace std
{
	template <class Type, bool val>
	struct hash<Storm::UniquePointer<Type, val>>
	{
		static std::size_t _Do_hash(const Storm::UniquePointer<Type, val>& keyval) noexcept(std::_Is_nothrow_hashable<typename Storm::UniquePointer<Type, val>::pointer>::value)
		{
			return std::hash<typename Storm::UniquePointer<Type, val>::UnderlyingPtrType>{}(static_cast<const Storm::UniquePointer<Type, val>::UnderlyingPtrType &>(keyval));
		}
	};
}
