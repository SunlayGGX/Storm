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

	template<class ContainerType, class Func>
	auto runParallel(ContainerType &container, Func &func)
		-> decltype(func(*std::begin(container), Storm::retrieveItemIndex(container, *std::begin(container))), void())
	{
#if STORM_USE_OPENMP
#		pragma omp parallel for
		for (int iter = 0; iter < container.size(); ++iter)
		{
			func(container[iter], iter);
		}
#else
		std::for_each(std::execution::par, std::begin(container), std::end(container), [&container, &func](auto &item)
		{
			func(item, Storm::retrieveItemIndex(container, item));
		});
#endif
	}

	template<class ContainerType, class Func>
	auto runParallel(ContainerType &container, Func &func)
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
}
