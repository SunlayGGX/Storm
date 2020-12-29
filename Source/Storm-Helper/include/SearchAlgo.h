#pragma once

#include "NonInstanciable.h"


namespace Storm
{
	class SearchAlgo : private Storm::NonInstanciable
	{
	private:
		template<class PtrType>
		static auto extract(PtrType &value, int) -> decltype(value == nullptr, Storm::SearchAlgo::extract(*value, 0))
		{
			if (value != nullptr)
			{
				return Storm::SearchAlgo::extract(*value, 0);
			}
			else
			{
				Storm::throwException<Storm::Exception>("Trying to extract a null value! It is forbidden!");
			}
		}

		template<class ValueType>
		static ValueType& extract(ValueType &value, void*)
		{
			return value;
		}

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
				Storm::throwException<Storm::Exception>(errorMsg);
			}
		}

	private:
		template<class KeyType, class Func, class ContainerType, class ... OthersContainerType>
		static bool executeOnObjectInContainerImpl(const KeyType &key, const Func &func, ContainerType &container, OthersContainerType &... others)
		{
			return
				Storm::SearchAlgo::executeOnObjectInContainerImpl(key, func, container) ||
				Storm::SearchAlgo::executeOnObjectInContainerImpl(key, func, others...);
		}

		template<class KeyType, class Func, class ContainerType>
		static bool executeOnObjectInContainerImpl(const KeyType &key, const Func &func, ContainerType &container)
		{
			if (const auto found = container.find(key); found != std::end(container))
			{
				func(Storm::SearchAlgo::extract(found->second, 0));
				return true;
			}

			return false;
		}

	public:
		template<class KeyType, class Func, class ... OthersContainerType>
		static void executeOnObjectInContainer(const KeyType &key, const Func &func, OthersContainerType &... allContainers)
		{
			if (!Storm::SearchAlgo::executeOnObjectInContainerImpl(key, func, allContainers...))
			{
				Storm::throwException<Storm::Exception>("Cannot find object with id " + Storm::toStdString(key) + " inside given maps. (" __FUNCSIG__ ")");
			}
		}

	private:
		template<class Func, class ContainerType, class ... OthersContainerType>
		static void executeOnContainerImpl(const Func &func, ContainerType &container, OthersContainerType &... others)
		{
			Storm::SearchAlgo::executeOnContainerImpl(func, container);
			Storm::SearchAlgo::executeOnContainerImpl(func, others...);
		}

		template<class Func, class ContainerType>
		static void executeOnContainerImpl(const Func &func, ContainerType &container)
		{
			for (auto &item : container)
			{
				func(Storm::SearchAlgo::extract(item.second, 0));
			}
		}

	public:
		template<class Func, class ... OthersContainerType>
		static void executeOnContainer(const Func &func, OthersContainerType &... allContainers)
		{
			Storm::SearchAlgo::executeOnContainerImpl(func, allContainers...);
		}
	};
}
