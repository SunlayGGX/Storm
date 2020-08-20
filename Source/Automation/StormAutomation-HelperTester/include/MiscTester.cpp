#include "MemoryHelper.h"
#include "ThrowException.h"
#include "ValueGuard.h"
#include "StringHijack.h"


TEST_CASE("MemoryHelper.ZeroMemories", "[classic]")
{
	int val[] = { 32, 64, 128 };
	Storm::ZeroMemories(val);

	CHECK(val[0] == 0);
	CHECK(val[1] == 0);
	CHECK(val[2] == 0);
}

TEST_CASE("throwException", "[classic]")
{
	try
	{
		Storm::throwException<std::exception>(__FUNCTION__);
		FAIL("No exception was thrown!");
	}
	catch (const std::exception &ex)
	{
		CHECK(std::string{ ex.what() } == __FUNCTION__);
	}
	catch (...)
	{
		FAIL("Exception not what we expected it to be!");
	}


	try
	{
		Storm::throwException<int>(777);
		FAIL("No exception was thrown!");
	}
	catch (int val)
	{
		CHECK(val == 777);
	}
	catch (...)
	{
		FAIL("Exception not what we expected it to be!");
	}
}

TEST_CASE("ValueGuard", "[classic]")
{
	std::string value = "toto";

	{
		Storm::ValueGuard guard{ value };
		REQUIRE(value == "toto");

		value = "titi";
		REQUIRE(value == "titi");
	}

	CHECK(value == "toto");
}

TEST_CASE("stringHijack", "[classic]")
{
	std::string someValue;
	someValue.reserve(64);

	REQUIRE(someValue.capacity() >= 64);
	REQUIRE(someValue.size() == 0);

	Storm::resize_hijack(someValue, 20);
	CHECK(someValue.capacity() >= 64);
	CHECK(someValue.size() == 20);

	Storm::resize_hijack(someValue, 64);
	CHECK(someValue.capacity() >= 64);
	CHECK(someValue.size() == 64);
}
