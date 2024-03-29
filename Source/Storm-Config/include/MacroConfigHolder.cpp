#include "MacroConfigHolder.h"
#include "ConfigManager.h"

#include "TimeHelper.h"
#include "StormPathHelper.h"

#include "Config/MacroTags.cs"

#pragma warning(push)
#pragma warning(disable:4702)
#	include <boost/property_tree/ptree.hpp>
#	include <boost/property_tree/xml_parser.hpp>
#pragma warning(pop)


namespace
{
	enum { k_macroTag = '$' };
	std::string makeFinalMacroKey(const std::string_view key)
	{
		std::string result;
		result.reserve(3 + key.size());
		result += static_cast<char>(k_macroTag);
		result += '[';
		result += key;
		result += ']';

		return result;
	}

	class MacroLogParserPolicy
	{
	public:
		template<class Policy, class MacroPairType>
		static auto parseAppending(std::string &inOutStr, const MacroPairType &macroPair) -> decltype(macroPair.first, macroPair.second, void())
		{
			// If heuristic was correctly guessed, then this will be ignored.
			inOutStr.reserve(inOutStr.size() + macroPair.first.size() + macroPair.second.size() + 16);

			inOutStr += "- ";
			inOutStr += macroPair.first;
			inOutStr += " equals to \"";
			inOutStr += macroPair.second;
			inOutStr += '"';
		}

		// Those 2 definitions are not mandatory, but allows more stability.
	public:
		// Force the separator in case we parse a macro container map, to use line break separator. Therefore, we're not dependent on the default implementation of toStdString anymore.
		static constexpr const char separator()
		{
			return '\n';
		}
		
		enum : std::size_t { k_expectedItemSize = 128 };
	};
}


Storm::MacroConfigHolder::MacroConfigHolder() :
	_lastHasResolved{ false }
{

}

void Storm::MacroConfigHolder::initialize()
{
	const Storm::ConfigManager &configMgr = Storm::ConfigManager::instance();
	const std::filesystem::path exePath = std::filesystem::path{ configMgr.getExePath() };
	const std::filesystem::path exeFolderPath = exePath.parent_path();

	const std::filesystem::path rootPath = Storm::StormPathHelper::findStormRootPath(exeFolderPath);

	const std::filesystem::path outputPath = rootPath / "Intermediate";

	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_StormExe, configMgr.getExePath());
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_StormFolderExe, exeFolderPath.string());
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_StormRoot, rootPath.string());
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_StormConfig, (rootPath / "Config").string());
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_StormResource, (rootPath / "Resource").string());
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_StormIntermediate, outputPath.string());
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_StormRecord, (outputPath / "Record").string());
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_StormStates, (outputPath / "States").string());
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_StormScripts, (outputPath / "Scripts").string());
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_StormDebug, (outputPath / "Debug").string());
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_StormArchive, (outputPath / "Archive").string());
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_DateTime, Storm::TimeHelper::getCurrentDateTime(false));
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_Date, Storm::TimeHelper::getCurrentDate());
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_PID, Storm::toStdString(configMgr.getCurrentPID()));
	this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_ComputerName, Storm::toStdString(configMgr.getComputerName()));

	if (std::filesystem::exists(outputPath))
	{
		this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_StormTmp, outputPath.string());
	}
	else
	{
		const std::filesystem::path tmpPath = std::filesystem::temp_directory_path() / "Storm";
		if (!std::filesystem::exists(tmpPath))
		{
			std::filesystem::create_directories(tmpPath);
		}
		this->registerMacroInternal(Storm::MacroTags::k_builtInMacroKey_StormTmp, tmpPath.string());
	}

	LOG_COMMENT << "Pre built-in macros registered:\n" << this->produceAllMacroLog();
}

bool Storm::MacroConfigHolder::read(const std::string &macroConfigFilePathStr)
{
	const std::filesystem::path macroConfigFilePath{ macroConfigFilePathStr };
	if (std::filesystem::is_regular_file(macroConfigFilePath))
	{
		if (macroConfigFilePath.extension() == ".xml")
		{
			boost::property_tree::ptree xmlTree;
			boost::property_tree::read_xml(macroConfigFilePathStr, xmlTree, boost::property_tree::xml_parser::no_comments);

			std::size_t macrosSizeBefore = _macros.size();

			const auto &macroTree = xmlTree.get_child("macros");

			std::vector<MacroKey> registeredMacro;
			registeredMacro.reserve(macroTree.size());

			for (const auto &macro : macroTree)
			{
				if (macro.first == "macro")
				{
					MacroKey macroKey = makeFinalMacroKey(macro.second.get<std::string>("<xmlattr>.key"));
					MacroValue macroValue = macro.second.get<std::string>("<xmlattr>.value");

					if (!macroKey.empty())
					{
						if (macroValue.find(macroKey) != std::string::npos)
						{
							Storm::throwException<Storm::Exception>(macroValue + " contains its own key " + macroKey + ". This is not allowed as this would generate infinite recursion!");
						}

						registeredMacro.emplace_back(_macros.emplace(std::move(macroKey), std::move(macroValue)).first->first);
					}
					else
					{
						LOG_WARNING << "A Macro with an empty key was detected, we will skip it";
					}
				}
				else
				{
					LOG_ERROR << "Unrecognizable macro xml tag (" << macro.first << "). Macro xml tag should be 'macro'";
				}
			}

			int watchdog = 0;
			do 
			{
				this->resolveInternalMacro();

				++watchdog;
				if (watchdog == 20)
				{
					Storm::throwException<Storm::Exception>(
						"Detected infinite recursion when resolving a macro.\n"
						"It means that a macro is referencing another that is referencing the first"
					);
				}

			} while (_lastHasResolved);

			const std::size_t registeredMacroCount = _macros.size() - macrosSizeBefore;
			if (registeredMacroCount > 0)
			{
				LOG_COMMENT <<
					"We have registered " << registeredMacroCount << " macros!\n"
					"Those were :\n" << this->produceMacroLog(registeredMacro);
			}
			else
			{
				LOG_COMMENT << "We haven't registered any macros!";
			}
			return true;
		}
		else
		{
			LOG_ERROR << "Macro config file at location " << macroConfigFilePathStr << " is not an xml file!";
		}
	}
	else
	{
		LOG_ERROR << "Macro config file at location " << macroConfigFilePathStr << " doesn't exists or isn't a file!";
	}

	return false;
}

const std::pair<const Storm::MacroConfigHolder::MacroKey, Storm::MacroConfigHolder::MacroValue>& Storm::MacroConfigHolder::registerMacroInternal(const std::string_view key, std::string value)
{
	return *_macros.insert_or_assign(makeFinalMacroKey(key), std::move(value)).first;
}

const std::string*const Storm::MacroConfigHolder::queryMacroValue(const std::string &key) const
{
	if (const auto found = _macros.find(makeFinalMacroKey(key)); found != std::end(_macros))
	{
		return &found->second;
	}

	return nullptr;
}

void Storm::MacroConfigHolder::registerMacro(const std::string &key, std::string value, bool shouldLog /*= true*/)
{
	const auto &macroPair = this->registerMacroInternal(key, std::move(value));
	if (shouldLog)
	{
		LOG_COMMENT << "We have registered the macro from code :\n" << Storm::toStdString<MacroLogParserPolicy>(macroPair);
	}
}

void Storm::MacroConfigHolder::resolveInternalMacro()
{
	for (auto &macro : _macros)
	{
		this->operator ()(macro.second);
	}
}

std::string Storm::MacroConfigHolder::produceMacroLog(const std::vector<MacroKey> &selectedMacros) const
{
	std::string result;
	result.reserve(selectedMacros.size() * MacroLogParserPolicy::k_expectedItemSize);

	const auto notFound = std::end(_macros);
	for (const MacroKey &macroToLog : selectedMacros)
	{
		if (const auto found = _macros.find(macroToLog); found != notFound)
		{
			result += Storm::toStdString<MacroLogParserPolicy>(found);
		}
		else
		{
			LOG_DEBUG_ERROR << "There is no macro named root " << macroToLog;
		}
	}

	return result;
}

std::string Storm::MacroConfigHolder::produceAllMacroLog() const
{
	return Storm::toStdString<MacroLogParserPolicy>(_macros);
}

bool Storm::MacroConfigHolder::hasKnownMacro(const std::string &inOutStr, std::size_t &outPosFound) const
{
	auto macroTagFound = inOutStr.find(static_cast<char>(k_macroTag));
	if (macroTagFound == std::string::npos)
	{
		return false;
	}

	for (const auto &macro : _macros)
	{
		if (outPosFound = inOutStr.find(macro.first, macroTagFound); outPosFound != std::string::npos)
		{
			return true;
		}
	}

	return false;
}


std::string Storm::MacroConfigHolder::makeMacroKey(const std::string_view value) const
{
	return makeFinalMacroKey(value);
}

void Storm::MacroConfigHolder::operator()(std::string &inOutStr) const
{
	_lastHasResolved = false;

	// If we suspect inOutStr to have a macro embedded in the string, then we would not enter the if and compute everything, otherwise we can just skip everything...
	std::string::size_type firstMacroTagFound = inOutStr.find(static_cast<char>(k_macroTag));
	if (firstMacroTagFound == std::string::npos)
	{
		return;
	}

	this->operator ()(inOutStr, firstMacroTagFound);
}

void Storm::MacroConfigHolder::operator()(std::string &inOutStr, const std::size_t beginSearchPos) const
{
	// In fact, this allows recursion because macro recursiveness was solved when they were registered.
	// Therefore no macro contains any other macros when we come here (outside of the macro registration). So we don't need to recurse here.
	for (const auto &macro : _macros)
	{
		std::size_t found;

		do
		{
			if (found = inOutStr.find(macro.first, beginSearchPos); found != std::string::npos)
			{
				_lastHasResolved = true;

				std::string substituted;
				const std::size_t macroKeySize = macro.first.size();
				substituted.reserve(inOutStr.size() - macroKeySize + macro.second.size());

				if (found != 0)
				{
					substituted += inOutStr.substr(0, found);
				}

				substituted += macro.second;
				substituted += inOutStr.substr(found + macroKeySize);

				inOutStr = std::move(substituted);
			}
		} while (found != std::string::npos);
	}
}

std::string Storm::MacroConfigHolder::operator()(const std::string &inStr) const
{
	std::string result = inStr;
	this->operator ()(result);
	return result;
}

