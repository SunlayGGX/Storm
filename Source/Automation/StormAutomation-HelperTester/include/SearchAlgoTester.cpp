#include "SearchAlgo.h"


namespace
{
	struct Obj
	{
		bool operator==(const std::string &val) const { return _val == val; }
		bool operator<(const std::string &val) const { return _val < val; }
		std::string _val;
	};

	template<bool shouldThrow, class ContainerType, class ValueOrPredType>
	void testThrowIfExist(const ContainerType &cont, const ValueOrPredType &valueOrPred)
	{
		constexpr std::string_view existValue = "It exists!";

		try
		{
			Storm::SearchAlgo::throwIfExist(cont, existValue, valueOrPred);

			if constexpr (shouldThrow)
			{
				FAIL("Storm::SearchAlgo::throwIfExist should have thrown, but it didn't!");
			}
		}
		catch (const std::exception &ex)
		{
			if constexpr (shouldThrow)
			{
				CHECK(existValue == ex.what());
			}
			else
			{
				FAIL("Storm::SearchAlgo::throwIfExist have thrown, but it shouldn't have!");
			}
		}
		catch (...)
		{
			if constexpr (shouldThrow)
			{
				FAIL("Storm::SearchAlgo::throwIfExist haven't thrown a std exception!");
			}
			else
			{
				FAIL("Storm::SearchAlgo::throwIfExist have thrown, but it shouldn't have!");
			}
		}
	}

	template<bool shouldThrow, class KeyType, class ValueType, class ... ContainerTypes>
	void testExecuteOnObjectInContainer(const KeyType &key, const ValueType &expected, const ContainerTypes &... containers)
	{
		try
		{
			bool executed = false;
			Storm::SearchAlgo::executeOnObjectInContainer(key, [&executed, &expected](const auto &found)
			{
				executed = true;
				CHECK(typeid(found) == typeid(expected));
				CHECK(Storm::toStdString(found) == Storm::toStdString(expected));
			}, containers...);

			if constexpr (shouldThrow)
			{
				FAIL("Storm::SearchAlgo::executeOnObjectInContainer should have thrown, but it didn't!");
			}
			else
			{
				if (executed)
				{
					SUCCEED("Storm::SearchAlgo::executeOnObjectInContainer executed its callback as expected.");
				}
				else
				{
					FAIL("Storm::SearchAlgo::executeOnObjectInContainer has not executed its callback.");
				}
			}
		}
		catch (const std::exception &)
		{
			if constexpr (shouldThrow)
			{
				SUCCEED("Storm::SearchAlgo::executeOnObjectInContainer have thrown an std exception as expected");
			}
			else
			{
				FAIL("Storm::SearchAlgo::executeOnObjectInContainer have thrown, but it shouldn't have!");
			}
		}
		catch (...)
		{
			if constexpr (shouldThrow)
			{
				FAIL("Storm::SearchAlgo::executeOnObjectInContainer haven't thrown a std exception!");
			}
			else
			{
				FAIL("Storm::SearchAlgo::executeOnObjectInContainer have thrown, but it shouldn't have!");
			}
		}
	}
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

TEST_CASE("SearchAlgo.throwIfExist.vector", "[classic]")
{
	const std::vector<std::string> strVect{ "toto", "titi", "tutu" };
	const std::vector<Obj> objVect{ Obj{ "toto" }, Obj{ "titi" }, Obj{ "tutu" } };

	testThrowIfExist<true>(strVect, "toto");
	testThrowIfExist<false>(strVect, "tata");

	testThrowIfExist<true>(objVect, "toto");
	testThrowIfExist<false>(objVect, "tata");

	testThrowIfExist<true>(objVect, [toCheck = std::string{ "toto" }](const Obj &val) { return val == toCheck; });
	testThrowIfExist<false>(objVect, [toCheck = std::string{ "tata" }](const Obj &val) { return val == toCheck; });
}

TEST_CASE("SearchAlgo.throwIfExist.map", "[classic]")
{
	const std::map<int, std::string> mapStr{ std::pair<int, std::string>{ 0, "toto" }, std::pair<int, std::string>{ 1, "titi" }, std::pair<int, std::string>{ -5, "tutu" } };
	const std::map<int, Obj> mapObj{ std::pair<int, Obj>{ 0, Obj{ "toto" } }, std::pair<int, Obj>{ 1, Obj{ "titi" } }, std::pair<int, Obj>{ -5, Obj{ "tutu" } } };

	testThrowIfExist<true>(mapStr, "toto");
	testThrowIfExist<false>(mapStr, "tata");

	testThrowIfExist<true>(mapObj, "toto");
	testThrowIfExist<false>(mapObj, "tata");

	testThrowIfExist<true>(mapObj, [toCheck = std::string{ "toto" }](const std::pair<int, Obj> &val) { return val.second == toCheck; });
	testThrowIfExist<false>(mapObj, [toCheck = std::string{ "tata" }](const std::pair<int, Obj> &val) { return val.second == toCheck; });
}

TEST_CASE("SearchAlgo.executeOnObjectInContainer.map", "[classic]")
{
	const std::map<int, std::string> mapStr{ std::pair<int, std::string>{ 0, "toto" }, std::pair<int, std::string>{ 1, "titi" }, std::pair<int, std::string>{ -5, "tutu" } };
	const std::map<int, std::wstring> mapWStr{ std::pair<int, std::wstring>{ 2, L"wtoto" }, std::pair<int, std::wstring>{ 4, L"wtiti" }, std::pair<int, std::wstring>{ -6, L"wtutu" } };

	testExecuteOnObjectInContainer<true>(-10000, "", mapStr, mapWStr);
	testExecuteOnObjectInContainer<false>(0, std::string{ "toto" }, mapStr, mapWStr);
	testExecuteOnObjectInContainer<false>(4, std::wstring{ L"wtiti" }, mapStr, mapWStr);
}
