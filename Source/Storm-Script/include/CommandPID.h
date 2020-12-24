#pragma once

#include "CommandItem.h"


namespace Storm
{
	class CommandPID : public Storm::CommandItem
	{
	public:
		CommandPID(std::string &&commandStr);

	public:
		void handleLogic(Storm::CommandLogicSupport &inOutParam) final override;

	private:
		std::vector<unsigned int> _pids;
	};
}
