#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class SearchAlgo : private Storm::NonInstanciable
	{
	private:
		template<class PairType, class ValueType>
		static auto compare(const ValueType &value, const PairType &pair, int) -> decltype(pair.second == value)
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

		template<class ContainerType, class ValueType, class MsgType>
		static auto searchIfExistImpl(const ContainerType &cont, const MsgType &errorMsg, const ValueType &value, int)
			-> decltype(Storm::SearchAlgo::compare(value, *std::begin(cont), 0), bool())
		{
			return std::find_if(std::begin(cont), std::end(cont), [&value](const auto &other)
			{
				return Storm::SearchAlgo::compare(value, other, 0);
			}) != std::end(cont);
		}

		template<class ContainerType, class PredicateType, class MsgType>
		static auto searchIfExistImpl(const ContainerType &cont, const MsgType &errorMsg, const PredicateType &predicate, void*)
			-> decltype(std::find_if(std::begin(cont), std::end(cont), predicate), bool())
		{
			return std::find_if(std::begin(cont), std::end(cont), predicate) != std::end(cont);
		}

	public:
		template<class ContainerType, class ValueOrPredicateType, class MsgType>
		static bool searchIfExist(const ContainerType &cont, const MsgType &errorMsg, const ValueOrPredicateType &valueOrPredicate)
		{
			return Storm::SearchAlgo::searchIfExistImpl(cont, errorMsg, valueOrPredicate, 0);
		}

		template<class ContainerType, class ValueOrPredicateType, class MsgType>
		static void throwIfExist(const ContainerType &cont, const MsgType &errorMsg, const ValueOrPredicateType &valueOrPredicate)
		{
			if (Storm::SearchAlgo::searchIfExistImpl(cont, errorMsg, valueOrPredicate, 0))
			{
				Storm::throwException<std::exception>(errorMsg);
			}
		}
	};
}
