#include "InvertPeriod.h"


namespace
{
	template<class ValueType>
	bool isAlmostEqual(ValueType val1, ValueType val2)
	{
		return std::abs(val1 - val2) < static_cast<ValueType>(0.00001);
	}
}


TEST_CASE("ChronoTester.InvertPeriod", "[classic]")
{
	CHECK(Storm::InvertPeriod<std::ratio<1, 1000>>::value == 1000.f);
	CHECK(Storm::InvertPeriod<std::ratio<1, 10>>::value == 10.f);
	CHECK(Storm::InvertPeriod<std::ratio<10, 1>>::value == (1.f / 10.f));
}

TEST_CASE("ChronoTester.ChronoHelper.toFps", "[classic]")
{
	CHECK(isAlmostEqual(Storm::ChronoHelper::toFps(std::chrono::milliseconds{ 1000 }), 1.f));
	CHECK(isAlmostEqual(Storm::ChronoHelper::toFps(std::chrono::milliseconds{ 10 }), 100.f));
	CHECK(isAlmostEqual(Storm::ChronoHelper::toFps(std::chrono::nanoseconds{ 16666667 }), 60.f));
	CHECK(isAlmostEqual(Storm::ChronoHelper::toFps(std::chrono::nanoseconds{ 33333333 }), 30.f));
}
