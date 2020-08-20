#include "SizeCounter.h"


TEST_CASE("SizeCounter", "[classic]")
{
	CHECK(Storm::SizeCounter<double>::value == 8);
	CHECK(Storm::SizeCounter<double, double, double, float, std::string>::value == 68);
	CHECK(Storm::SizeCounter<double, double, float, std::string>::value == 60);
	CHECK(Storm::SizeCounter<int32_t[24]>::value == 96);
}
