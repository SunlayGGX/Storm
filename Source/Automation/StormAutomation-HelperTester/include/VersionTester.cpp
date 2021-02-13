#include "Version.h"
#include "UniversalString.h"


TEST_CASE("Version.string", "[classic]")
{
	CHECK(Storm::Version{ 1, 2, 3 } == std::string{ "1.2.3" });
	CHECK(std::string{ "1.2.3" } == Storm::Version{ 1, 2, 3 });

	CHECK(Storm::Version{ std::string{ "1.2.3" } } == std::string{ "1.2.3" });
	CHECK(std::string{ "1.2.3" } == Storm::Version{ std::string{ "1.2.3" } });

	CHECK(Storm::Version{ std::string{ "1.2.3" } } == Storm::Version{ 1, 2, 3 });
}

TEST_CASE("Version.string_view", "[classic]")
{
	CHECK(Storm::Version{ 1, 2, 3 } == std::string_view{ "1.2.3" });
	CHECK(std::string_view{ "1.2.3" } == Storm::Version{ 1, 2, 3 });

	CHECK(Storm::Version{ std::string_view{ "1.2.3" } } == std::string_view{ "1.2.3" });
	CHECK(std::string{ "1.2.3" } == Storm::Version{ std::string_view{ "1.2.3" } });

	CHECK(Storm::Version{ std::string_view{ "1.2.3" } } == Storm::Version{ 1, 2, 3 });
}

TEST_CASE("Version.char[]", "[classic]")
{
	CHECK(Storm::Version{ 1, 2, 3 } == "1.2.3");
	CHECK("1.2.3" == Storm::Version{ 1, 2, 3 });

	CHECK(Storm::Version{ "1.2.3" } == "1.2.3");
	CHECK("1.2.3"== Storm::Version{ "1.2.3" });

	CHECK(Storm::Version{ "1.2.3" } == Storm::Version{ 1, 2, 3 });
}

TEST_CASE("Version.superiority", "[classic]")
{
	CHECK(Storm::Version{ 1, 1, 1 } > Storm::Version{ 1, 1, 0 });
	CHECK(Storm::Version{ 1, 1, 1 } > Storm::Version{ 1, 0, 1 });
	CHECK(Storm::Version{ 1, 1, 1 } > Storm::Version{ 0, 1, 1 });
	CHECK(Storm::Version{ 1, 1, 1 } > Storm::Version{ 1, 0, 0 });

	CHECK(Storm::Version{ 1, 1, 0 } < Storm::Version{ 1, 1, 1 });
	CHECK(Storm::Version{ 1, 0, 1 } < Storm::Version{ 1, 1, 1 });
	CHECK(Storm::Version{ 0, 1, 1 } < Storm::Version{ 1, 1, 1 });
	CHECK(Storm::Version{ 1, 0, 0 } < Storm::Version{ 1, 1, 1 });

	REQUIRE(Storm::Version{} == Storm::Version{ 0, 0, 0 });
	CHECK(Storm::Version{ 1 } > Storm::Version{});
	CHECK(Storm::Version{ 1, 1 } > Storm::Version{ 1 });
	CHECK(Storm::Version{ 1, 1, 1 } > Storm::Version{ 1, 1 });
}

TEST_CASE("Version.stringConversion", "[classic]")
{
	CHECK(Storm::toStdString(Storm::Version{ 1, 2, 3 }) == std::string{ "1.2.3" });
	CHECK(Storm::toStdString(Storm::Version{ std::string{ "1.2.3" } }) == std::string{ "1.2.3" });
}
