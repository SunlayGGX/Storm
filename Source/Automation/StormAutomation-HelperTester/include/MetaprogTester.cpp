#include "SizeCounter.h"

namespace
{
	class Object
	{
	private:
		int32_t _1;
		double _2;
		double* _3;
		uint64_t _4;
	};
}

TEST_CASE("SizeCounter", "[classic]")
{
	CHECK(Storm::SizeCounter<double>::value == 8);
	CHECK(Storm::SizeCounter<double, double, double, float, Object>::value == 60);
	CHECK(Storm::SizeCounter<double, double, float, Object>::value == 52);
	CHECK(Storm::SizeCounter<int32_t[24]>::value == 96);
}
