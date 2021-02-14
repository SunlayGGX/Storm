#include "StringAlgo.h"



TEST_CASE("StringAlgo.split.string", "[classic]")
{
	std::vector<std::string> result;

	CHECK(
		Storm::StringAlgo::split(result, "super toto is not super toto but toto", Storm::StringAlgo::makeSplitPredicate<std::string>(' ', "toto")) ==
		std::vector<std::string>{ "super", "is", "not", "super", "but" }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split(result, "     super toto is not super toto but toto   ", Storm::StringAlgo::makeSplitPredicate<std::string>(' ', "toto")) ==
		std::vector<std::string>{ "super", "is", "not", "super", "but" }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split(result, "     super toto is not super toto but toto   ", Storm::StringAlgo::makeSplitPredicate<std::string>(' ', "toto")) ==
		std::vector<std::string>{ "super", "is", "not", "super", "but" }
	);
	// no clear.

	CHECK(
		Storm::StringAlgo::split(result, "totooootototot", Storm::StringAlgo::makeSplitPredicate<std::string>(' ', "toto")) ==
		std::vector<std::string>{ "super", "is", "not", "super", "but", "ooo", "tot" }
	);
	result.clear();

	const char literalWithTerminalString[] = "superman\0\0is not \0 \0toto";

	CHECK(
		Storm::StringAlgo::split(result, std::string{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makeSplitPredicate<std::string>("")) ==
		std::vector<std::string>{ "superman", "is not ", " ", "toto" }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split(result, std::string{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makeSplitPredicate<std::string>('\0')) ==
		std::vector<std::string>{ "superman", "is not ", " ", "toto" }
	);

	result.clear();

	CHECK(
		Storm::StringAlgo::split(result, "#####\n Enab:superman\n#####\ntoto\ntiti()\n#####\n#####\n\nsuperman:toto(5)\n", Storm::StringAlgo::makeSplitPredicate<std::string>("#####")) ==
		std::vector<std::string>{ "\n Enab:superman\n", "\ntoto\ntiti()\n", "\n", "\n\nsuperman:toto(5)\n" }
	);
}

#define STORM_TEXT(Text) Text

TEST_CASE("StringAlgo.split.string_view", "[classic]")
{
	using StringType = std::string_view;

	std::vector<StringType> result;

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("super toto is not super toto but toto"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	// no clear.

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("totooootototot"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but"), STORM_TEXT("ooo"), STORM_TEXT("tot") }
	);
	result.clear();

	const StringType::value_type literalWithTerminalString[] = STORM_TEXT("superman\0\0is not \0 \0toto");

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(""))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT('\0'))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);

	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("#####\n Enab:superman\n#####\ntoto\ntiti()\n#####\n#####\n\nsuperman:toto(5)\n"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT("#####"))) ==
		std::vector<StringType>{ STORM_TEXT("\n Enab:superman\n"), STORM_TEXT("\ntoto\ntiti()\n"), STORM_TEXT("\n"), STORM_TEXT("\n\nsuperman:toto(5)\n") }
	);
}

#undef STORM_TEXT
#define STORM_TEXT(Text) L##Text

TEST_CASE("StringAlgo.split.wstring", "[classic]")
{
	using StringType = std::wstring;

	std::vector<StringType> result;

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("super toto is not super toto but toto"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	// no clear.

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("totooootototot"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but"), STORM_TEXT("ooo"), STORM_TEXT("tot") }
	);
	result.clear();

	const StringType::value_type literalWithTerminalString[] = STORM_TEXT("superman\0\0is not \0 \0toto");

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(""))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT('\0'))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);

	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("#####\n Enab:superman\n#####\ntoto\ntiti()\n#####\n#####\n\nsuperman:toto(5)\n"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT("#####"))) ==
		std::vector<StringType>{ STORM_TEXT("\n Enab:superman\n"), STORM_TEXT("\ntoto\ntiti()\n"), STORM_TEXT("\n"), STORM_TEXT("\n\nsuperman:toto(5)\n") }
	);
}

#undef STORM_TEXT
#define STORM_TEXT(Text) L##Text

TEST_CASE("StringAlgo.split.wstring_view", "[classic]")
{
	using StringType = std::wstring_view;

	std::vector<StringType> result;

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("super toto is not super toto but toto"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	// no clear.

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("totooootototot"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but"), STORM_TEXT("ooo"), STORM_TEXT("tot") }
	);
	result.clear();

	const StringType::value_type literalWithTerminalString[] = STORM_TEXT("superman\0\0is not \0 \0toto");

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(""))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT('\0'))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);

	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("#####\n Enab:superman\n#####\ntoto\ntiti()\n#####\n#####\n\nsuperman:toto(5)\n"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT("#####"))) ==
		std::vector<StringType>{ STORM_TEXT("\n Enab:superman\n"), STORM_TEXT("\ntoto\ntiti()\n"), STORM_TEXT("\n"), STORM_TEXT("\n\nsuperman:toto(5)\n") }
	);
}


#undef STORM_TEXT
#define STORM_TEXT(Text) u8##Text

TEST_CASE("StringAlgo.split.u8string", "[classic]")
{
	using StringType = std::u8string;

	std::vector<StringType> result;

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("super toto is not super toto but toto"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	// no clear.

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("totooootototot"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but"), STORM_TEXT("ooo"), STORM_TEXT("tot") }
	);
	result.clear();

	const StringType::value_type literalWithTerminalString[] = STORM_TEXT("superman\0\0is not \0 \0toto");

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(""))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT('\0'))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);

	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("#####\n Enab:superman\n#####\ntoto\ntiti()\n#####\n#####\n\nsuperman:toto(5)\n"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT("#####"))) ==
		std::vector<StringType>{ STORM_TEXT("\n Enab:superman\n"), STORM_TEXT("\ntoto\ntiti()\n"), STORM_TEXT("\n"), STORM_TEXT("\n\nsuperman:toto(5)\n") }
	);
}

#undef STORM_TEXT
#define STORM_TEXT(Text) U##Text

TEST_CASE("StringAlgo.split.u32string", "[classic]")
{
	using StringType = std::u32string;

	std::vector<StringType> result;

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("super toto is not super toto but toto"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("     super toto is not super toto but toto   "), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but") }
	);
	// no clear.

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("totooootototot"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(' '), STORM_TEXT("toto"))) ==
		std::vector<StringType>{ STORM_TEXT("super"), STORM_TEXT("is"), STORM_TEXT("not"), STORM_TEXT("super"), STORM_TEXT("but"), STORM_TEXT("ooo"), STORM_TEXT("tot") }
	);
	result.clear();

	const StringType::value_type literalWithTerminalString[] = STORM_TEXT("superman\0\0is not \0 \0toto");

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT(""))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);
	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, StringType{ literalWithTerminalString, Storm::StringAlgo::extractSize(literalWithTerminalString) }, Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT('\0'))) ==
		std::vector<StringType>{ STORM_TEXT("superman"), STORM_TEXT("is not "), STORM_TEXT(" "), STORM_TEXT("toto") }
	);

	result.clear();

	CHECK(
		Storm::StringAlgo::split<StringType>(result, STORM_TEXT("#####\n Enab:superman\n#####\ntoto\ntiti()\n#####\n#####\n\nsuperman:toto(5)\n"), Storm::StringAlgo::makeSplitPredicate<StringType>(STORM_TEXT("#####"))) ==
		std::vector<StringType>{ STORM_TEXT("\n Enab:superman\n"), STORM_TEXT("\ntoto\ntiti()\n"), STORM_TEXT("\n"), STORM_TEXT("\n\nsuperman:toto(5)\n") }
	);
}

#undef STORM_TEXT


TEST_CASE("StringAlgo.split.mix", "[classic]")
{
	std::vector<std::string_view> stringViewResult;
	CHECK(
		Storm::StringAlgo::split(stringViewResult, std::string{ "#####\n Enab:superman\n#####\ntoto\ntiti()\n#####\n#####\n\nsuperman:toto(5)\n" }, Storm::StringAlgo::makeSplitPredicate("#####")) ==
		std::vector<std::string_view>{ "\n Enab:superman\n", "\ntoto\ntiti()\n", "\n", "\n\nsuperman:toto(5)\n" }
	);
}


TEST_CASE("StringAlgo.replace.string", "[classic]")
{
	using StringType = std::string;
	using StringViewType = std::string_view;

	CHECK(
		Storm::StringAlgo::replaceAllCopy("super toto is not super toto but toto", "ti", Storm::StringAlgo::makeReplacePredicate<StringType>("toto", ' ')) ==
		"supertititiistinottisupertititibuttiti"
	);

	CHECK(
		Storm::StringAlgo::replaceAllCopy("\t superman\t\n is\tnot\ttoto.\n\r\n\n", " ", Storm::StringAlgo::makeReplacePredicate<StringType>('\t', '\n', "\r\n")) ==
		"  superman   is not toto.   "
	);

	CHECK(Storm::StringAlgo::replaceAllCopy("toto", "", Storm::StringAlgo::makeReplacePredicate<StringType>('o')) == "tt");
	CHECK(Storm::StringAlgo::replaceAllCopy("toto", StringType{ "" }, Storm::StringAlgo::makeReplacePredicate<StringType>('o')) == "tt");
	CHECK(Storm::StringAlgo::replaceAllCopy("toto", StringViewType{ "" }, Storm::StringAlgo::makeReplacePredicate<StringType>('o')) == "tt");
}

#define STORM_TEXT(Text) U##Text

TEST_CASE("StringAlgo.replace.u32string", "[classic]")
{
	using StringType = std::u32string;
	using StringViewType = std::u32string_view;

	CHECK(
		Storm::StringAlgo::replaceAllCopy(STORM_TEXT("super toto is not super toto but toto"), STORM_TEXT("ti"), Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT("toto"), STORM_TEXT(' '))) ==
		STORM_TEXT("supertititiistinottisupertititibuttiti")
	);

	CHECK(
		Storm::StringAlgo::replaceAllCopy(STORM_TEXT("\t superman\t\n is\tnot\ttoto.\n\r\n\n"), STORM_TEXT(" "), Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT('\t'), STORM_TEXT('\n'), STORM_TEXT("\r\n"))) ==
		STORM_TEXT("  superman   is not toto.   ")
	);

	CHECK(Storm::StringAlgo::replaceAllCopy(STORM_TEXT("toto"), STORM_TEXT(""), Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT('o'))) == STORM_TEXT("tt"));

	CHECK(Storm::StringAlgo::replaceAllCopy(STORM_TEXT("toto"), StringType{ STORM_TEXT("") }, Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT('o'))) == STORM_TEXT("tt"));
	CHECK(Storm::StringAlgo::replaceAllCopy(STORM_TEXT("toto"), StringViewType{ STORM_TEXT("") }, Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT('o'))) == STORM_TEXT("tt"));
}

#undef STORM_TEXT
#define STORM_TEXT(Text) u8##Text

TEST_CASE("StringAlgo.replace.u8string", "[classic]")
{
	using StringType = std::u8string;
	using StringViewType = std::u8string_view;

	CHECK(
		Storm::StringAlgo::replaceAllCopy(STORM_TEXT("super toto is not super toto but toto"), STORM_TEXT("ti"), Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT("toto"), STORM_TEXT(' '))) ==
		STORM_TEXT("supertititiistinottisupertititibuttiti")
	);

	CHECK(
		Storm::StringAlgo::replaceAllCopy(STORM_TEXT("\t superman\t\n is\tnot\ttoto.\n\r\n\n"), STORM_TEXT(" "), Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT('\t'), STORM_TEXT('\n'), STORM_TEXT("\r\n"))) ==
		STORM_TEXT("  superman   is not toto.   ")
	);

	CHECK(Storm::StringAlgo::replaceAllCopy(STORM_TEXT("toto"), STORM_TEXT(""), Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT('o'))) == STORM_TEXT("tt"));

	CHECK(Storm::StringAlgo::replaceAllCopy(STORM_TEXT("toto"), StringType{ STORM_TEXT("") }, Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT('o'))) == STORM_TEXT("tt"));
	CHECK(Storm::StringAlgo::replaceAllCopy(STORM_TEXT("toto"), StringViewType{ STORM_TEXT("") }, Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT('o'))) == STORM_TEXT("tt"));
}

#undef STORM_TEXT
#define STORM_TEXT(Text) L##Text

TEST_CASE("StringAlgo.replace.wstring", "[classic]")
{
	using StringType = std::wstring;
	using StringViewType = std::wstring_view;

	CHECK(
		Storm::StringAlgo::replaceAllCopy(STORM_TEXT("super toto is not super toto but toto"), STORM_TEXT("ti"), Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT("toto"), STORM_TEXT(' '))) ==
		STORM_TEXT("supertititiistinottisupertititibuttiti")
	);

	CHECK(
		Storm::StringAlgo::replaceAllCopy(STORM_TEXT("\t superman\t\n is\tnot\ttoto.\n\r\n\n"), STORM_TEXT(" "), Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT('\t'), STORM_TEXT('\n'), STORM_TEXT("\r\n"))) ==
		STORM_TEXT("  superman   is not toto.   ")
	);

	CHECK(Storm::StringAlgo::replaceAllCopy(STORM_TEXT("toto"), STORM_TEXT(""), Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT('o'))) == STORM_TEXT("tt"));

	CHECK(Storm::StringAlgo::replaceAllCopy(STORM_TEXT("toto"), StringType{ STORM_TEXT("") }, Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT('o'))) == STORM_TEXT("tt"));
	CHECK(Storm::StringAlgo::replaceAllCopy(STORM_TEXT("toto"), StringViewType{ STORM_TEXT("") }, Storm::StringAlgo::makeReplacePredicate<StringType>(STORM_TEXT('o'))) == STORM_TEXT("tt"));
}

#undef STORM_TEXT

