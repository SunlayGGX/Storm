#include "CommandItem.h"

#include "CommandPID.h"
#include "CommandEnable.h"

#include "CommandLogicSupport.h"
#include "CommandKeyword.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "StringAlgo.h"

#include <boost\algorithm\string\case_conv.hpp>

namespace
{
	namespace details
	{

		template<Storm::CommandKeyword forbiddenKey, Storm::CommandKeyword ... remainingForbiddenKeys>
		struct ForbiddenKeyEnsurer
		{
		public:
			static constexpr void doEnsure(const std::string_view &allForbiddenKeys, const Storm::CommandKeyword key)
			{
				details::ForbiddenKeyEnsurer<forbiddenKey>::doEnsure(allForbiddenKeys, key);
				details::ForbiddenKeyEnsurer<remainingForbiddenKeys...>::doEnsure(allForbiddenKeys, key);
			}
		};

		template<Storm::CommandKeyword forbiddenKey>
		struct ForbiddenKeyEnsurer<forbiddenKey>
		{
		public:
			static constexpr void doEnsure(const std::string_view &allForbiddenKeys, const Storm::CommandKeyword key)
			{
				if (key == forbiddenKey)
				{
					Storm::throwException<Storm::Exception>(std::string_view{
						"We detected a forbidden key!\n"
						"We shouldn't use the key you see in the template parameter inside the logic of the current command (see details in the callstack).\n"
						"All forbidden keys are : " } + allForbiddenKeys
					);
				}
			}
		};
	}

	template<Storm::CommandKeyword ... forbiddenKeys>
	constexpr void ensureNoConflictingKey_Exec(const std::string_view &allForbiddenKeys, const Storm::CommandLogicSupport &param)
	{
		if constexpr (sizeof...(forbiddenKeys) == 0)
		{
			return;
		}
		else
		{
			for (const Storm::CommandKeyword alreadyExecutedKey : param._executedKeys)
			{
				details::ForbiddenKeyEnsurer<forbiddenKeys...>::doEnsure(allForbiddenKeys, alreadyExecutedKey);
			}
		}
	}

#pragma warning(push)
#pragma warning(disable: 4003) // If __VA_ARGS__ is empty (no keys considered as conflicting), then STORM_STRINGIFY will complain it doesn't have enough arg and will be ignored (which is what we want)...
#	define ensureNoConflictingKey(param, ...) ensureNoConflictingKey_Exec<__VA_ARGS__>(STORM_STRINGIFY(__VA_ARGS__) "", param)
}


Storm::CommandItem::CommandItem(const Storm::CommandKeyword keyword) :
	_keyword{ keyword }
{

}

Storm::CommandItem::~CommandItem() = default;

Storm::CommandKeyword Storm::CommandItem::getKeyword() const noexcept
{
	return _keyword;
}

void Storm::CommandItem::baseDirectHandlePostLogic(Storm::CommandLogicSupport &inOutParam) const
{
	inOutParam._executedKeys.emplace_back(_keyword);
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
	ensureNoConflictingKey(inOutParam);

	if (inOutParam._canExecute)
	{
		const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
		inOutParam._canExecute &= std::find(std::begin(_pids), std::end(_pids), configMgr.getCurrentPID()) != std::end(_pids);
	}
}


//**************************************************//
//*************** Command Enable *******************//
//**************************************************//

Storm::CommandEnable::CommandEnable(std::string &&commandStr) :
	Storm::CommandItem{ Storm::CommandKeyword::Enabled }
{
	if (commandStr.empty())
	{
		Storm::throwException<Storm::Exception>("Enable command cannot be empty!");
	}

	boost::to_lower(commandStr);
	if (
		commandStr == "true" ||
		commandStr == "1" ||
		commandStr == "on"
		)
	{
		_enabledValue = true;
	}
	else if (
		commandStr == "false" ||
		commandStr == "0" ||
		commandStr == "off"
		)
	{
		_enabledValue = false;
	}
	else
	{
		Storm::throwException<Storm::Exception>("Unknown command value '" + commandStr + "'. How are we supposed to parse it in the current context ?");
	}
}

void Storm::CommandEnable::handleLogic(Storm::CommandLogicSupport &inOutParam)
{
	ensureNoConflictingKey(inOutParam, Storm::CommandKeyword::Enabled);

	if (inOutParam._canExecute)
	{
		inOutParam._canExecute &= _enabledValue;
	}
}

#pragma warning(pop)