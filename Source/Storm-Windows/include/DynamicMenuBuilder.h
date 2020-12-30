#pragma once


namespace Storm
{
	class DynamicMenuBuilder
	{
	public:
		using MenuFunctionCallback = std::function<void()>;

	public:
		~DynamicMenuBuilder();

	public:
		void appendMenu(const HMENU parent, const std::wstring &menuContentName, Storm::DynamicMenuBuilder::MenuFunctionCallback &&callback);
		bool operator()(const UINT commandID);

	private:
		std::vector<std::pair<UINT, std::vector<Storm::DynamicMenuBuilder::MenuFunctionCallback>>> _menuCallbacks;
	};
}
