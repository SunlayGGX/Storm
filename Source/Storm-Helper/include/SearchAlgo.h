#pragma once

#include "NonInstanciable.h"
#include "ThrowException.h"


namespace Storm
{
	class SearchAlgo : private Storm::NonInstanciable
	{
	private:
		template<class PairType, class ValueType>
		static auto compare(const ValueType &value, const PairType &pair, int) -> decltype(value == pair.second)
		{
			return value == pair.second;
		}

		template<class OtherType, class ValueType>
		static auto compare(const ValueType &value, const OtherType &other, void*) -> decltype(other == value)
		{
			return value == other;
		}

		template<class OtherType, class ValueType>
		static auto compare(const ValueType &value, const OtherType &other, ...) -> decltype(other == nullptr, Storm::SearchAlgo::compare(value, *other, 0))
		{
			if (other != nullptr)
			{
				return Storm::SearchAlgo::compare(value, *other, 0);
			}

			return false;
		}

		template<class ContainerType, class ValueType>
		static auto searchIfExistImpl(const ContainerType &cont, const ValueType &value, int)
			-> decltype(Storm::SearchAlgo::compare(value, *std::begin(cont), 0), bool())
		{
			return std::find_if(std::begin(cont), std::end(cont), [&value](const auto &other)
			{
				return Storm::SearchAlgo::compare(value, other, 0);
			}) != std::end(cont);
		}

		template<class ContainerType, class PredicateType>
		static auto searchIfExistImpl(const ContainerType &cont, const PredicateType &predicate, void*)
			-> decltype(std::find_if(std::begin(cont), std::end(cont), predicate), bool())
		{
			return std::find_if(std::begin(cont), std::end(cont), predicate) != std::end(cont);
		}

	public:
		template<class ContainerType, class ValueOrPredicateType>
		static bool searchIfExist(const ContainerType &cont, const ValueOrPredicateType &valueOrPredicate)
		{
			return Storm::SearchAlgo::searchIfExistImpl(cont, valueOrPredicate, 0);
		}

		template<class ContainerType, class ValueOrPredicateType, class MsgType>
		static void throwIfExist(const ContainerType &cont, const MsgType &errorMsg, const ValueOrPredicateType &valueOrPredicate)
		{
			if (Storm::SearchAlgo::searchIfExistImpl(cont, valueOrPredicate, 0))
			{
				Storm::throwException<std::exception>(errorMsg);
			}
		}
	};
}
