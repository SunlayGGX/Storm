#include "CommandParser.h"

#include "ScriptObject.h"
#include "CommandKeyword.h"
#include "CommandLogicSupport.h"
#include "CommandItem.h"
#include "CommandPID.h"
#include "CommandEnable.h"

#include "StringAlgo.h"

#include <boost\algorithm\string\case_conv.hpp>
#include <boost\algorithm\string\trim.hpp>


namespace
{
	constexpr std::string_view getCommandDelimitor()
	{
		return "#####";
	}

	Storm::CommandKeyword parseKeyword(std::string &token)
	{
		boost::to_lower(token);
		boost::trim(token);

		if (token == "pid")
		{
			return Storm::CommandKeyword::PID;
		}
		else if (token == "enabled")
		{
			return Storm::CommandKeyword::Enabled;
		}
		else
		{
			Storm::throwException<Storm::Exception>("Unknown keyword '" + token + "'");
		}
	}

	template<class ErrorMsgType, class ErrorStackTraceType>
	void addError(std::string &inOutError, const ErrorMsgType &errorMsg, const ErrorStackTraceType &errorStackTrace, std::size_t &errorCount)
	{
		const std::string errorCountStr = std::to_string(errorCount++);

		inOutError.reserve(inOutError.size() + errorMsg.size() + errorStackTrace.size() + errorCountStr.size() + 128);
		inOutError += "Error ";
		inOutError += errorCountStr;
		inOutError += " : Command parsing error happened :\n";
		inOutError += errorMsg;
		inOutError += ".\n";
		inOutError += errorStackTrace;
		inOutError += "\n\n\n";
	}
}



std::vector<Storm::ScriptObject> Storm::CommandParser::parse(std::string &&totalScriptCommand)
{
	std::vector<Storm::ScriptObject> result;

	Storm::StringAlgo::replaceAllCopy(totalScriptCommand, '\n', Storm::StringAlgo::makeReplacePredicate<std::string_view>('\x1', '\x2'));

	std::string::size_type firstDelimitorFound = totalScriptCommand.find(getCommandDelimitor());
	if (firstDelimitorFound == std::string::npos)
	{
		result.emplace_back(std::move(totalScriptCommand));
	}
	else
	{
		std::string firstScript = totalScriptCommand.substr(0, firstDelimitorFound);
		if (!firstScript.empty() && !std::all_of(std::begin(firstScript), std::end(firstScript), [](const char ch) 
		{
			return
				ch == '\n' ||
				ch == '\t' ||
				ch == ' '  ||
				ch == '\r' ||
				ch == '\0'
				;
		}))
		{
			result.emplace_back(std::move(firstScript));
		}

		std::vector<std::string_view> scriptChunks;
		Storm::StringAlgo::split(scriptChunks, std::string_view{ &totalScriptCommand[firstDelimitorFound], totalScriptCommand.size() - firstDelimitorFound }, Storm::StringAlgo::makeSplitPredicate<std::string_view>(getCommandDelimitor()));

		bool isEnteringCommands = true;
		std::string errors;
		std::size_t errorIter = 0;

		std::vector<std::unique_ptr<Storm::CommandItem>> _currentStackedCommand;

		for (const std::string_view &script : scriptChunks)
		{
			if (isEnteringCommands)
			{
				std::string commandPart = Storm::StringAlgo::replaceAllCopy(script, ' ', Storm::StringAlgo::makeReplacePredicate<std::string_view>('\t', ','));

				std::vector<std::string> commandLines;
				Storm::StringAlgo::split(commandLines, commandPart, Storm::StringAlgo::makeSplitPredicate<std::string_view>('\n', ';', "\r\n"));

				for (std::string &command : commandLines)
				{
					std::vector<std::string> commandElements;
					Storm::StringAlgo::split(commandElements, command, Storm::StringAlgo::makeSplitPredicate<std::string_view>(':'));

					try
					{
						if (commandElements.size() < 3)
						{
							boost::trim(commandElements[1]);

							Storm::CommandKeyword keyword = parseKeyword(commandElements[0]);
							switch (keyword)
							{
							case Storm::CommandKeyword::PID:
								_currentStackedCommand.emplace_back(std::make_unique<Storm::CommandPID>(std::move(commandElements[1])));
								break;

							case Storm::CommandKeyword::Enabled:
								_currentStackedCommand.emplace_back(std::make_unique<Storm::CommandEnable>(std::move(commandElements[1])));
								break;

							default:
								__assume(false);
							}
						}
						else
						{
							Storm::throwException<Storm::Exception>("Too many tokens detected in command '" + command + "' !");
						}
					}
					catch (const Storm::Exception &ex)
					{
						const std::string_view errorMsg = ex.what();
						const std::string_view errorStackTrace = ex.stackTrace();

						addError(errors, errorMsg, errorStackTrace, errorIter);
					}
				}
			}
			else
			{
				// If we have errors, then it isn't worth continuing. Like compile errors, we would just gather all errors, then stop without finishing.
				if (errors.empty())
				{
					Storm::CommandLogicSupport param;
					param._executedKeys.reserve(_currentStackedCommand.size());
					
					for (const auto &command : _currentStackedCommand)
					{
						command->handleLogic(param);
						command->baseDirectHandlePostLogic(param);
					}

					if (param._canExecute)
					{
						result.emplace_back(Storm::toStdString(script));
					}
				}

				_currentStackedCommand.clear();
			}

			isEnteringCommands = !isEnteringCommands;
		}

		// If we came here, then we have specified a command, but no script comes after...
		// Normally, after we finished registering the current script body, the variable should be true to be ready to enter command,
		// but here, we stopped with a command ready but no script.
		if (!isEnteringCommands)
		{
			constexpr std::string_view errorMsg{ "No script followed the last command!" };
			const std::string errorStackTrace = Storm::obtainStackTrace(false);

			addError(errors, errorMsg, errorStackTrace, errorIter);
		}

		if (!errors.empty())
		{
			// Remove the 3 last line breaks at the end.
			int toRemove = 3;
			while (toRemove)
			{
				errors.pop_back();
				--toRemove;
			}

			Storm::throwException<Storm::Exception>(errors);
		}
	}

	return result;
}
