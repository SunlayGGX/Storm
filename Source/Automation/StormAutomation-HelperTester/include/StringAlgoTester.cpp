#include "StringAlgo.h"



TEST_CASE("StringAlgo.split.string", "[classic]")
{
	std::vector<std::string> result;

	CHECK(
		Storm::StringAlgo::split(result, "super toto is not super toto but toto", Storm::StringAlgo::makePredicate<std::string>(' ', "toto")) ==
		std::vector<std::string>{ "super", "is", "not", "super", "but" }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split(result, "     super toto is not super toto but toto   ", Storm::StringAlgo::makePredicate<std::string>(' ', "toto")) ==
		std::vector<std::string>{ "super", "is", "not", "super", "but" }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split(result, "     super toto is not super toto but toto   ", Storm::StringAlgo::makePredicate<std::string>(' ', "toto")) ==
		std::vector<std::string>{ "super", "is", "not", "super", "but" }
	);
	// no clear.

	CHECK(
		Storm::StringAlgo::split(result, "totooootototot", Storm::StringAlgo::makePredicate<std::string>(' ', "toto")) ==
		std::vector<std::string>{ "super", "is", "not", "super", "but", "ooo", "tot" }
	);
	result.clear();

	const char literalWithTerminalString[] = "superman\0\0is not \0 \0toto";

	CHECK(
		Storm::StringAlgo::split(result, std::string{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makePredicate<std::string>("")) ==
		std::vector<std::string>{ "superman", "is not ", " ", "toto" }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split(result, std::string{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makePredicate<std::string>('\0')) ==
		std::vector<std::string>{ "superman", "is not ", " ", "toto" }
	);
}

#define STORM_TEXT(Text) Text

TEST_CASE("StringAlgo.split.string_view", "[classic]")
{
	using StringType = std::string_view;

	std::vector<StringType> result;

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("super toto is not super toto but toto"), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	// no clear.

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("totooootototot"), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but"), STORM_TEXT("ooo"), STORM_TEXT("tot") }
	);
	result.clear();

	const StringType::value_type literalWithTerminalString[] = STORM_TEXT("superman\0\0is not \0 \0toto");

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(""))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT('\0'))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
}

#undef STORM_TEXT
#define STORM_TEXT(Text) L##Text

TEST_CASE("StringAlgo.split.wstring", "[classic]")
{
	using StringType = std::wstring;

	std::vector<StringType> result;

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("super toto is not super toto but toto"), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	// no clear.

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("totooootototot"), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but"), STORM_TEXT("ooo"), STORM_TEXT("tot") }
	);
	result.clear();

	const StringType::value_type literalWithTerminalString[] = STORM_TEXT("superman\0\0is not \0 \0toto");

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(""))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT('\0'))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
}

#undef STORM_TEXT
#define STORM_TEXT(Text) L##Text

TEST_CASE("StringAlgo.split.wstring_view", "[classic]")
{
	using StringType = std::wstring_view;

	std::vector<StringType> result;

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("super toto is not super toto but toto"), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	// no clear.

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("totooootototot"), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but"), STORM_TEXT("ooo"), STORM_TEXT("tot") }
	);
	result.clear();

	const StringType::value_type literalWithTerminalString[] = STORM_TEXT("superman\0\0is not \0 \0toto");

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(""))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT('\0'))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
}


#undef STORM_TEXT
#define STORM_TEXT(Text) u8##Text

TEST_CASE("StringAlgo.split.u8string", "[classic]")
{
	using StringType = std::u8string;

	std::vector<StringType> result;

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("super toto is not super toto but toto"), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	// no clear.

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("totooootototot"), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but"), STORM_TEXT("ooo"), STORM_TEXT("tot") }
	);
	result.clear();

	const StringType::value_type literalWithTerminalString[] = STORM_TEXT("superman\0\0is not \0 \0toto");

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(""))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT('\0'))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
}

#undef STORM_TEXT
#define STORM_TEXT(Text) U##Text

TEST_CASE("StringAlgo.split.u32string", "[classic]")
{
	using StringType = std::u32string;

	std::vector<StringType> result;

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("super toto is not super toto but toto"), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	// no clear.

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("totooootototot"), Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but"), STORM_TEXT("ooo"), STORM_TEXT("tot") }
	);
	result.clear();

	const StringType::value_type literalWithTerminalString[] = STORM_TEXT("superman\0\0is not \0 \0toto");

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT(""))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makePredicate<StringType>(STORM_TEXT('\0'))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
}

#undef STORM_TEXT
