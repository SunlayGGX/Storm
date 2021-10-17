#pragma once

#include "MacroConfig.h"


namespace Storm
{
	template<class ItemType>
	std::size_t retrieveItemIndex(const std::vector<std::remove_cv_t<ItemType>> &container, const ItemType &item)
	{
		// Works only because std::vector is contiguous in memory.
		return &item - &container[0];
	}

	template<class Type>
	__forceinline auto defaultItem(int) -> decltype(Type{ 0 })
	{
		return Type{ 0 };
	}

	template<>
	__forceinline Storm::Vector3 defaultItem<Storm::Vector3>(int)
	{
		return Storm::Vector3::Zero();
	}

	template<class Type>
	__forceinline auto defaultItem(void*) -> decltype(Type{})
	{
		return Type{};
	}

	// Don't implement, they are like declval, this is just to trick the compiler
	template<class ContainerType> auto extractContainerType(const ContainerType &cont, int) -> decltype(*std::begin(cont)->second);
	template<class ContainerType> auto extractContainerType(const ContainerType &cont, void*) -> decltype(std::begin(cont)->second);
	template<class ContainerType> auto extractContainerType(const ContainerType &cont, ...) -> decltype(*std::begin(cont));

	template<bool useOpenMPWhenEnabled = false, class ContainerType, class Func>
	auto runParallel(ContainerType &container, Func &&func)
		-> decltype(func(*std::begin(container), Storm::retrieveItemIndex(container, *std::begin(container))), void())
	{
#if STORM_USE_OPENMP
		if constexpr (useOpenMPWhenEnabled)
		{
			const int64_t containerSz = static_cast<int64_t>(container.size());

#			pragma omp parallel default(shared)
#			pragma omp for schedule(static)
			for (int64_t iter = 0; iter < containerSz; ++iter)
			{
				func(container[iter], iter);
			}

			return;
		}
#endif

		std::for_each(std::execution::par, std::begin(container), std::end(container), [&container, &func](auto &item)
		{
			func(item, Storm::retrieveItemIndex(container, item));
		});
	}

	template<class ContainerType, class Func>
	auto runParallel(ContainerType &container, Func &&func)
		-> decltype(func(*std::begin(container)), void())
	{
		std::for_each(std::execution::par, std::begin(container), std::end(container), func);
	}

#if _WIN32
	__forceinline const unsigned int retrieveParallelPolicyExecThreadCount()
	{
		return __std_parallel_algorithms_hw_threads() * 4;
	}
#endif

	template<class ContainerType, class ValueType>
	auto reduceParallel(const ContainerType &container, const ValueType &value)
	{
		return std::reduce(std::execution::par, std::begin(container), std::end(container), value);
	}

	template<class ContainerType>
	auto reduceParallel(const ContainerType &container)
	{
		return Storm::reduceParallel(container, Storm::defaultItem<std::remove_cvref_t<decltype(Storm::extractContainerType(container, 0))>>(0));
	}
}
