#pragma once

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
	class UniquePointer : private std::unique_ptr<ValueType, Storm::details::Destroyer<ValueType>>
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

	public:
		UniquePointer(ValueType* ptr) :
			UnderlyingPtrType{ ptr, Storm::details::Destroyer<ValueType>{} }
		{}
	};

	template<class ValueType>
	class UniquePointer<ValueType, false> : private std::unique_ptr<ValueType>
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
	};
}
