#pragma once

#include "CommandItem.h"


namespace Storm
{
	class CommandEnable : public Storm::CommandItem
	{
	public:
		CommandEnable(std::string &&commandStr);

	public:
		void handleLogic(Storm::CommandLogicSupport &inOutParam) final override;

	private:
		bool _enabledValue;
	};
}
