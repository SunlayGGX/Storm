#include "MacroConfig.h"
#include "ConfigManager.h"

#include "TimeHelper.h"
#include "StormPathHelper.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


namespace
{
	enum { k_macroTag = '$' };
	std::string makeFinalMacroKey(const std::string &key)
	{
		return std::string{ static_cast<char>(k_macroTag) } + '[' + key + ']';
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


Storm::MacroConfig::MacroConfig() :
	_lastHasResolved{ false }
{

}

void Storm::MacroConfig::initialize()
{
	const Storm::ConfigManager &configMgr = Storm::ConfigManager::instance();
	const std::filesystem::path exePath = std::filesystem::path{ configMgr.getExePath() };
	const std::filesystem::path exeFolderPath = exePath.parent_path();

	const std::filesystem::path rootPath = Storm::StormPathHelper::findStormRootPath(exeFolderPath);

	const std::filesystem::path outputPath = rootPath / "Intermediate";

	this->registerMacroInternal("StormExe", configMgr.getExePath());
	this->registerMacroInternal("StormFolderExe", exeFolderPath.string());
	this->registerMacroInternal("StormRoot", rootPath.string());
	this->registerMacroInternal("StormConfig", (rootPath / "Config").string());
	this->registerMacroInternal("StormResource", (rootPath / "Resource").string());
	this->registerMacroInternal("StormIntermediate", outputPath.string());
	this->registerMacroInternal("StormRecord", (outputPath / "Record").string());
	this->registerMacroInternal("StormStates", (outputPath / "States").string());
	this->registerMacroInternal("StormScripts", (outputPath / "Scripts").string());
	this->registerMacroInternal("DateTime", Storm::TimeHelper::getCurrentDateTime(false));
	this->registerMacroInternal("Date", Storm::TimeHelper::getCurrentDate());
	this->registerMacroInternal("PID", Storm::toStdString(configMgr.getCurrentPID()));

	if (std::filesystem::exists(outputPath))
	{
		this->registerMacroInternal("StormTmp", outputPath.string());
	}
	else
	{
		const std::filesystem::path tmpPath = std::filesystem::temp_directory_path() / "Storm";
		if (!std::filesystem::exists(tmpPath))
		{
			std::filesystem::create_directories(tmpPath);
		}
		this->registerMacroInternal("StormTmp", tmpPath.string());
	}

	LOG_COMMENT << "Pre built-in macros registered:\n" << this->produceAllMacroLog();
}

bool Storm::MacroConfig::read(const std::string &macroConfigFilePathStr)
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
							Storm::throwException<Storm::StormException>(macroValue + " contains its own key " + macroKey + ". This is not allowed as this would generate infinite recursion!");
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
					Storm::throwException<Storm::StormException>(
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

const std::pair<const Storm::MacroConfig::MacroKey, Storm::MacroConfig::MacroValue>& Storm::MacroConfig::registerMacroInternal(const std::string &key, std::string value)
{
	return *_macros.emplace(makeFinalMacroKey(key), std::move(value)).first;
}

const std::string*const Storm::MacroConfig::queryMacroValue(const std::string &key) const
{
	if (const auto found = _macros.find(makeFinalMacroKey(key)); found != std::end(_macros))
	{
		return &found->second;
	}

	return nullptr;
}

void Storm::MacroConfig::registerMacro(const std::string &key, std::string value, bool shouldLog /*= true*/)
{
	const auto &macroPair = this->registerMacroInternal(key, std::move(value));
	if (shouldLog)
	{
		LOG_COMMENT << "We have registered the macro from code :\n" << Storm::toStdString<MacroLogParserPolicy>(macroPair);
	}
}

void Storm::MacroConfig::resolveInternalMacro()
{
	for (auto &macro : _macros)
	{
		this->operator ()(macro.second);
	}
}

std::string Storm::MacroConfig::produceMacroLog(const std::vector<MacroKey> &selectedMacros) const
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

std::string Storm::MacroConfig::produceAllMacroLog() const
{
	return Storm::toStdString<MacroLogParserPolicy>(_macros);
}

void Storm::MacroConfig::operator()(std::string &inOutStr) const
{
	_lastHasResolved = false;

	// If we suspect inOutStr to have a macro embedded in the string, then we would not enter the if and compute everything, otherwise we can just skip everything...
	if (inOutStr.find(static_cast<char>(k_macroTag)) == std::string::npos)
	{
		return;
	}

	for (const auto &macro : _macros)
	{
		std::size_t found;

		do 
		{
			if (found = inOutStr.find(macro.first); found != std::string::npos)
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

std::string Storm::MacroConfig::operator()(const std::string &inStr) const
{
	std::string result = inStr;
	this->operator ()(result);
	return result;
}

