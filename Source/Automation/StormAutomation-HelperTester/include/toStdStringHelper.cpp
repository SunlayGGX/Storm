#include "UniversalString.h"


TEST_CASE("toStdString", "[classic]")
{
	CHECK(Storm::toStdString(3) == "3");
	CHECK(Storm::toStdString(true) == "true");
	CHECK(Storm::toStdString(false) == "false");
	CHECK(Storm::toStdString(std::vector<int>{ 5, 6, 7, 9, -1 }) == "5,6,7,9,-1");
	CHECK(Storm::toStdString(std::map<int, std::string>{ { 5, "toto" }, { 6, "titi" }, { -1, "tutu" } }) == "tutu,toto,titi");
	CHECK(Storm::toStdString<Storm::DebugPolicy>(std::map<int, std::string>{ { 5, "toto" }, { 6, "titi" }, { -1, "tutu" } }) == "{ -1,tutu },{ 5,toto },{ 6,titi }");
	CHECK(Storm::toStdString(std::string{ "toto" }) == "toto");
	CHECK(Storm::toStdString(std::wstring{ L"toto" }) == "toto");
	CHECK(Storm::toStdString("toto") == "toto");
	CHECK(Storm::toStdString(L"toto") == "toto");
	
	std::string valStr{ "toto" };
	CHECK(Storm::toStdString(valStr) == "toto");

	const std::string valStrC{ "toto" };
	CHECK(Storm::toStdString(valStrC) == "toto");

	std::string valStr2 = Storm::toStdString(std::string{ "toto" });
	CHECK(valStr2 == "toto");

	const char* valLitType = "toto";
	CHECK(Storm::toStdString(valLitType) == "toto");


	int val = 6;
	int* valPtr = &val;
	int** valPtrPtr = &valPtr;
	CHECK(Storm::toStdString(val) == "6");
	CHECK(Storm::toStdString(valPtrPtr) == "6");
	CHECK(Storm::toStdString(&valPtrPtr) == "6");
	CHECK(Storm::toStdString(nullptr) == "");
	CHECK(Storm::toStdString<Storm::DebugPolicy>(nullptr) == "null");

	std::string* nullPtr = nullptr;
	CHECK(Storm::toStdString<Storm::DebugPolicy>(nullPtr) == "null");
}
