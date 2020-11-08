#include "UniversalString.h"


namespace
{
	struct Item;
	struct Item2
	{
	public:
		operator std::string() const { return "-" + _val1 + "-" + Storm::toStdString(_val2) + "-" + Storm::toStdString(_val3); }

	public:
		std::string _val1;
		const Item* _val2;
		const Item*const _val3 = nullptr;
	};

	struct Item
	{
	public:
		operator std::string() const { return _val1 + '_' + Storm::toStdString(_val2) + '/' + Storm::toStdString(_val3); }

	public:
		std::string _val1;
		const Item* _val2;
		const Item2* _val3;
	};

	class Policy1
	{
	public:
		template<class Policy>
		static std::string parse(const Item &item)
		{
			return item._val1 + '|' + Storm::toStdString<Policy>(item._val2) + '|' + Storm::toStdString<Policy>(item._val3);
		}
	};

	class Policy2 : public Storm::DebugPolicy
	{
	public:
		template<class Policy>
		static std::string parse(const Item &item)
		{
			return item._val1 + '|' + Storm::toStdString<Policy>(item._val2) + '|' + Storm::toStdString<Policy>(item._val3);
		}
	};

	class Policy3 : public Storm::DebugPolicy
	{
	public:
		template<class Policy>
		static void parseAppending(std::string &inOutStr, const Item &item)
		{
			inOutStr += item._val1;
			inOutStr += '|';
			inOutStr += Storm::toStdString<Policy>(item._val2);
			inOutStr += '|';
			inOutStr += Storm::toStdString<Policy>(item._val3);
		}
	};

	class Policy6 : public Policy2
	{
	public:
		using Policy2::parse;

		template<class Policy>
		static std::string parse(const Item2 &item)
		{
			return item._val1 + '|' + Storm::toStdString<Policy>(item._val2) + '|' + Storm::toStdString<Policy>(item._val3);
		}
	};
}


TEST_CASE("toStdString", "[classic]")
{
	CHECK(Storm::toStdString(3) == "3");
	CHECK(Storm::toStdString(true) == "true");
	CHECK(Storm::toStdString(false) == "false");
	CHECK(Storm::toStdString(std::vector<int>{ 5, 6, 7, 9, -1 }) == "5\n6\n7\n9\n-1");
	CHECK(Storm::toStdString(std::map<int, std::string>{ { 5, "toto" }, { 6, "titi" }, { -1, "tutu" } }) == "tutu\ntoto\ntiti");
	CHECK(Storm::toStdString<Storm::DebugPolicy>(std::map<int, std::string>{ { 5, "toto" }, { 6, "titi" }, { -1, "tutu" } }) == "{ -1,tutu }\n{ 5,toto }\n{ 6,titi }");
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


TEST_CASE("toStdString.Policy", "[classic]")
{
	class Policy4
	{
	public:
		static std::string parsePolicyAgnostic(const Item &item)
		{
			return
				item._val1 +
				"||" + (item._val2 ? Storm::toStdString<Policy4>(*item._val2) : "pointerNull") +
				"||" + (item._val3 ? Storm::toStdString<Policy4>(*item._val3) : "pointerNull");
		}
	};

	class Policy5 : public Policy4
	{
	public:
		using Policy4::parsePolicyAgnostic;
		static std::string parsePolicyAgnostic(const Item2 &item)
		{
			return 
				item._val1 + "xxxxx" +
				"||" + (item._val2 ? Storm::toStdString<Policy5>(*item._val2) : "NULL") +
				"||" + (item._val3 ? Storm::toStdString<Policy5>(*item._val3) : "NULL");
		}
	};

	const Item item1{ ._val1 = "toto", ._val2 = nullptr, ._val3 = nullptr };
	const Item2 other{ ._val1 = "superman", ._val2 = &item1 };
	const Item item2{ ._val1 = "titi", ._val2 = &item1, ._val3 = &other };
	const Item2 other2{ ._val1 = "superman", ._val2 = &item2 };

	const std::vector<Item> array = { {
		Item{ ._val1 = "tata", ._val2 = &item1, ._val3 = &other },
		Item{ ._val1 = "tutu", ._val2 = nullptr, ._val3 = &other },
		Item{ ._val1 = "tete", ._val2 = nullptr, ._val3 = nullptr },
		Item{ ._val1 = "tartempion", ._val2 = &item2, ._val3 = nullptr },
	} };

	CHECK(Storm::toStdString(item1) == "toto_/");
	CHECK(Storm::toStdString(item2) == "titi_toto_//-superman-toto_/-");

	CHECK(Storm::toStdString<Policy1>(item1) == "toto||");
	CHECK(Storm::toStdString<Policy1>(item2) == "titi|toto|||-superman-toto_/-");

	CHECK(Storm::toStdString<Policy2>(item1) == "toto|null|null");
	CHECK(Storm::toStdString<Policy2>(item2) == "titi|toto|null|null|-superman-toto_/-");

	CHECK(Storm::toStdString<Policy3>(item1) == "toto|null|null");
	CHECK(Storm::toStdString<Policy3>(item2) == "titi|toto|null|null|-superman-toto_/-");

	CHECK(Storm::toStdString<Policy4>(item1) == "toto||pointerNull||pointerNull");
	CHECK(Storm::toStdString<Policy4>(item2) == "titi||toto||pointerNull||pointerNull||-superman-toto_/-");
	CHECK(Storm::toStdString<Policy5>(item2) == "titi||toto||pointerNull||pointerNull||-superman-toto_/-"); // Here, this is a bad uses that we acknowledge, we shouldn't use a parsePolicyAgnostic with inheritage, or the Policy injection will be lost along the way, like demonstrated here.
	CHECK(Storm::toStdString<Policy5>(other2) == "supermanxxxxx||titi||toto||pointerNull||pointerNull||-superman-toto_/-||NULL"); // Likewise.
	CHECK(Storm::toStdString<Policy6>(item2) == "titi|toto|null|null|superman|toto|null|null|null");

	CHECK(Storm::toStdString<Policy1>(array) == "tata|toto|||-superman-toto_/-\ntutu||-superman-toto_/-\ntete||\ntartempion|titi|toto|||-superman-toto_/-|");
	CHECK(Storm::toStdString<Policy2>(array) == "tata|toto|null|null|-superman-toto_/-\ntutu|null|-superman-toto_/-\ntete|null|null\ntartempion|titi|toto|null|null|-superman-toto_/-|null");
	CHECK(Storm::toStdString<Policy3>(array) == "tata|toto|null|null|-superman-toto_/-\ntutu|null|-superman-toto_/-\ntete|null|null\ntartempion|titi|toto|null|null|-superman-toto_/-|null");
	CHECK(Storm::toStdString<Policy4>(array) == "tata||toto||pointerNull||pointerNull||-superman-toto_/-\ntutu||pointerNull||-superman-toto_/-\ntete||pointerNull||pointerNull\ntartempion||titi||toto||pointerNull||pointerNull||-superman-toto_/-||pointerNull");
	CHECK(Storm::toStdString<Policy5>(array) == "tata||toto||pointerNull||pointerNull||-superman-toto_/-\ntutu||pointerNull||-superman-toto_/-\ntete||pointerNull||pointerNull\ntartempion||titi||toto||pointerNull||pointerNull||-superman-toto_/-||pointerNull");
	CHECK(Storm::toStdString<Policy6>(array) == "tata|toto|null|null|superman|toto|null|null|null\ntutu|null|superman|toto|null|null|null\ntete|null|null\ntartempion|titi|toto|null|null|superman|toto|null|null|null|null");
}
