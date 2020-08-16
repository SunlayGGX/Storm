#include "UniversalString.h"

#include "Vector3.h"



TEST_CASE("toStdString.Vector3", "[classic]")
{
	CHECK(Storm::toStdString(Storm::Vector3{ Storm::Vector3::Zero() }) == "{ 0.000000, 0.000000, 0.000000 }");
	CHECK(Storm::toStdString(Storm::Vector3{ -10.f, 5.f, 2.f }) == "{ -10.000000, 5.000000, 2.000000 }");
}
