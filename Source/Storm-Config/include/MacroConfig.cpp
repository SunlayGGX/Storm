#include "MacroConfig.h"
#include "ConfigManager.h"

#include "ThrowException.h"
#include "TimeHelper.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


namespace
{
	enum { k_macroTag = '$' };
	std::string makeFinalMacroKey(const std::string &key)
	{
		return std::string{} + static_cast<char>(k_macroTag) + '[' + key + ']';
	}

	
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
	const std::filesystem::path rootPath = exeFolderPath.parent_path();
	const std::filesystem::path outputPath = rootPath / "Intermediate";

	_macros[makeFinalMacroKey("StormExe")] = configMgr.getExePath();
	_macros[makeFinalMacroKey("StormFolderExe")] = exeFolderPath.string();
	_macros[makeFinalMacroKey("StormRoot")] = rootPath.string();
	_macros[makeFinalMacroKey("StormConfig")] = (rootPath / "Config").string();
	_macros[makeFinalMacroKey("StormResource")] = (rootPath / "Resource").string();
	_macros[makeFinalMacroKey("StormIntermediate")] = outputPath.string();
	_macros[makeFinalMacroKey("DateTime")] = Storm::TimeHelper::getCurrentDateTime(false);
	_macros[makeFinalMacroKey("Date")] = Storm::TimeHelper::getCurrentDate();

	if (std::filesystem::exists(outputPath))
	{
		_macros[makeFinalMacroKey("StormTmp")] = outputPath.string();
	}
	else
	{
		const std::filesystem::path tmpPath = std::filesystem::temp_directory_path() / "Storm";
		if (!std::filesystem::exists(tmpPath))
		{
			std::filesystem::create_directories(tmpPath);
		}
		_macros[makeFinalMacroKey("StormTmp")] = tmpPath.string();
	}
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

			for (const auto &macro : macroTree)
			{
				if (macro.first == "macro")
				{
					std::string macroKey = makeFinalMacroKey(macro.second.get<std::string>("<xmlattr>.key"));
					std::string macroValue = macro.second.get<std::string>("<xmlattr>.value");

					if (!macroKey.empty())
					{
						if (macroValue.find(macroKey) != std::string::npos)
						{
							Storm::throwException<std::exception>(macroValue + " contains its own key " + macroKey + ". This is not allowed as this would generate infinite recursion!");
						}

						_macros.emplace(std::move(macroKey), std::move(macroValue));
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
				for (auto &macro : _macros)
				{
					this->operator ()(macro.second);
				}

				++watchdog;
				if (watchdog == 20)
				{
					Storm::throwException<std::exception>(
						"Detected infinite recursion when resolving a macro.\n"
						"It means that a macro is referencing another that is referencing the first"
					);
				}

			} while (_lastHasResolved);

			LOG_COMMENT << "We have registered " << _macros.size() - macrosSizeBefore << " macros!";
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

const std::string*const Storm::MacroConfig::queryMacroValue(const std::string &key) const
{
	if (const auto found = _macros.find(makeFinalMacroKey(key)); found != std::end(_macros))
	{
		return &found->second;
	}

	return nullptr;
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

