#include "SizeCounter.h"
#include "CorrectSettingChecker.h"
#include "NameExtractor.h"

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

TEST_CASE("CorrectSettingChecker", "[classic]")
{
	enum class EnumTest
	{
		Val1,
		Val2,
		Val3,
		Val4,
		Val5,
	};

	using CorrectSettingChecker = Storm::CorrectSettingChecker<EnumTest>;

	CHECK(CorrectSettingChecker::check<EnumTest::Val1, EnumTest::Val2, EnumTest::Val3>(EnumTest::Val4) == false);
	CHECK(CorrectSettingChecker::check<EnumTest::Val1>(EnumTest::Val4) == false);
	CHECK(CorrectSettingChecker::check<EnumTest::Val4>(EnumTest::Val4) == true);
	CHECK(CorrectSettingChecker::check<EnumTest::Val1, EnumTest::Val2, EnumTest::Val4>(EnumTest::Val4) == true);
	CHECK(CorrectSettingChecker::check<EnumTest::Val1, EnumTest::Val2, EnumTest::Val3, EnumTest::Val4>(EnumTest::Val4) == true);
}

TEST_CASE("NameExtractor.extractTypeNameFromType", "[classic]")
{
	CHECK(Storm::NameExtractor::extractTypeNameFromType("") == "");
	CHECK(Storm::NameExtractor::extractTypeNameFromType("superman") == "superman");
	CHECK(Storm::NameExtractor::extractTypeNameFromType("Storm::MegaClass") == "MegaClass");
	CHECK(Storm::NameExtractor::extractTypeNameFromType("Storm::SomeNamespace::SomeNamespace2::details::TotoStruct") == "TotoStruct");
}
