#include "CommandItem.h"

#include "CommandPID.h"

#include "CommandLogicSupport.h"
#include "CommandKeyword.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "StringAlgo.h"


Storm::CommandItem::CommandItem(const Storm::CommandKeyword keyword) :
	_keyword{ keyword }
{

}

Storm::CommandItem::~CommandItem() = default;

Storm::CommandKeyword Storm::CommandItem::getKeyword() const noexcept
{
	return _keyword;
}


//**************************************************//
//*************** Command PID **********************//
//**************************************************//

Storm::CommandPID::CommandPID(std::string &&commandStr) :
	Storm::CommandItem{ Storm::CommandKeyword::PID }
{
	if (commandStr.empty())
	{
		Storm::throwException<Storm::Exception>("PID command cannot be empty!");
	}

	std::string error;

	std::vector<std::string> splitted;
	splitted.reserve(std::count(std::begin(commandStr), std::end(commandStr), ' ') + 1);

	Storm::StringAlgo::split(splitted, commandStr, Storm::StringAlgo::makeSplitPredicate<std::string>(' '));

	char* endValidator;
	_pids.reserve(splitted.size());
	for (const std::string &pidStr : splitted)
	{
		const char*const pidCStr = pidStr.c_str();

		unsigned int pid = strtoul(pidCStr, &endValidator, 10);
		if (endValidator == (pidCStr + pidStr.size()))
		{
			_pids.emplace_back(pid);
		}
		else
		{
			error += pidStr + " cannot be converted!";
		}
	}
}

void Storm::CommandPID::handleLogic(Storm::CommandLogicSupport &inOutParam)
{
	if (inOutParam._canExecute)
	{
		const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
		inOutParam._canExecute &= std::find(std::begin(_pids), std::end(_pids), configMgr.getCurrentPID()) != std::end(_pids);
	}
}
