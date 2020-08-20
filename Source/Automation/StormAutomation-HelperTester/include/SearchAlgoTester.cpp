#include "SearchAlgo.h"


namespace
{
	struct Obj
	{
		bool operator==(const std::string &val) const { return _val == val; }
		bool operator<(const std::string &val) const { return _val < val; }
		std::string _val;
	};
}

TEST_CASE("SearchAlgo.searchIfExist.vector", "[classic]")
{
	const std::vector<std::string> strVect{ "toto", "titi", "tutu" };
	const std::vector<Obj> objVect{ Obj{ "toto" }, Obj{ "titi" }, Obj{ "tutu" } };

	CHECK(Storm::SearchAlgo::searchIfExist(strVect, "toto") == true);
	CHECK(Storm::SearchAlgo::searchIfExist(strVect, "tata") == false);

	CHECK(Storm::SearchAlgo::searchIfExist(objVect, "toto") == true);
	CHECK(Storm::SearchAlgo::searchIfExist(objVect, "tata") == false);

	CHECK(Storm::SearchAlgo::searchIfExist(objVect, [toCheck = std::string{ "toto" }](const Obj &val) { return val == toCheck; }) == true);
	CHECK(Storm::SearchAlgo::searchIfExist(objVect, [toCheck = std::string{ "tata" }](const Obj &val) { return val == toCheck; }) == false);
}

TEST_CASE("SearchAlgo.searchIfExist.map", "[classic]")
{
	const std::map<int, std::string> mapStr{ std::pair<int, std::string>{ 0, "toto" }, std::pair<int, std::string>{ 1, "titi" }, std::pair<int, std::string>{ -5, "tutu" } };
	const std::map<int, Obj> mapObj{ std::pair<int, Obj>{ 0, Obj{ "toto" } }, std::pair<int, Obj>{ 1, Obj{ "titi" } }, std::pair<int, Obj>{ -5, Obj{ "tutu" } } };

	CHECK(Storm::SearchAlgo::searchIfExist(mapStr, "toto") == true);
	CHECK(Storm::SearchAlgo::searchIfExist(mapStr, "tata") == false);

	CHECK(Storm::SearchAlgo::searchIfExist(mapObj, "toto") == true);
	CHECK(Storm::SearchAlgo::searchIfExist(mapObj, "tata") == false);

	CHECK(Storm::SearchAlgo::searchIfExist(mapObj, [toCheck = std::string{ "toto" }](const std::pair<int, Obj> &val) { return val.second == toCheck; }) == true);
	CHECK(Storm::SearchAlgo::searchIfExist(mapObj, [toCheck = std::string{ "tata" }](const std::pair<int, Obj> &val) { return val.second == toCheck; }) == false);
}

