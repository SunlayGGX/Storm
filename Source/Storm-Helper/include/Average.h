#pragma once

#include "MemoryHelper.h"


namespace Storm
{
	class AverageBase
	{
	protected:
		template<class ValueType>
		static auto getZeroValue(int) -> decltype(ValueType::Zero()) // For Eigen based objects
		{
			return ValueType::Zero();
		}

		template<class ValueType>
		static auto getZeroValue(void*) -> decltype(static_cast<ValueType>(0))
		{
			return static_cast<ValueType>(0);
		}
		
		template<class ValueType> static auto extractUnderlyingType(int) -> decltype(ValueType::Scalar());
		template<class ValueType> static auto extractUnderlyingType(void*) -> decltype(static_cast<ValueType>(0));
	};

	template<std::size_t scanWidth> class MovingAverageTraits {};
	class CummulativeMovingAverageTraits {};

	template<class ValueType, class ComputationTraits = Storm::CummulativeMovingAverageTraits>
	class Average
	{
		template<class BadType>
		constexpr static int forceJITCompileError()
		{
			STORM_COMPILE_ERROR("Invalid Average ComputationTraits!");
		}

		enum { jitCompileError = forceJITCompileError<ComputationTraits>() };
	};

	template<class ValueType, std::size_t scanWidth>
	class Average<ValueType, Storm::MovingAverageTraits<scanWidth>> : private Storm::AverageBase
	{
	private:
		using UnderlyingType = decltype(Storm::AverageBase::extractUnderlyingType<ValueType>(0));

	public:
		Average() :
			_averageCached{ Storm::AverageBase::getZeroValue<ValueType>(0) },
			_index{ 0 }
		{
			std::fill(std::begin(_buffer), std::end(_buffer), _averageCached);
		}

		Average(const Average &) = default;
		Average(Average &&) = default;

	public:
		void addValue(const ValueType &value)
		{
			_buffer[_index] = value;
			const std::size_t nextIndex = (_index + static_cast<std::size_t>(1)) % scanWidth; // Next is in fact the older element (the one "out" next).

			// Simple moving average (boxcar filter)
			float val = 1.f / static_cast<UnderlyingType>(scanWidth);
			_averageCached += ((value - _buffer[nextIndex]) * val);

			_index = nextIndex;
		}

		const ValueType& getAverage() const noexcept
		{
			return _averageCached;
		}

		void reset()
		{
			_averageCached = Storm::AverageBase::getZeroValue<ValueType>(0);
			std::fill(std::begin(_buffer), std::end(_buffer), _averageCached);
		}

	private:
		ValueType _averageCached;
		ValueType _buffer[scanWidth];
		std::size_t _index;
	};

	template<class ValueType>
	class Average<ValueType, Storm::CummulativeMovingAverageTraits> : private Storm::AverageBase
	{
	public:
		Average() :
			_average{ Storm::AverageBase::getZeroValue<ValueType>(0) },
			_count{ 0 }
		{}

		Average(const Average &) = default;
		Average(Average &&) = default;

	public:
		void addValue(const ValueType &value)
		{
			_average += (value - _average) / static_cast<UnderlyingType>(_count + 1);
			++_count;
		}

		const ValueType& getAverage() const noexcept
		{
			return _average;
		}

		void reset()
		{
			_average = Storm::AverageBase::getZeroValue<ValueType>(0);
			_count = 0;
		}

	private:
		ValueType _average;
		std::size_t _count;
	};
}
