#pragma once


namespace Storm
{
	enum class CommandKeyword;

	struct CommandLogicSupport
	{
	public:
		bool _canExecute = true;
		std::vector<Storm::CommandKeyword> _executedKeys;
	};
}
