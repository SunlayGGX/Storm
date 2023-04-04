#include "ExporterConfigManager.h"

#include "ExportMode.h"
#include "ExportType.h"

#include "StringAlgo.h"
#include "StormPathHelper.h"

#include <boost\program_options.hpp>
#include <boost\algorithm\string\case_conv.hpp>


namespace
{
	template<class Type, class CommandlineContainer>
	void extract(const CommandlineContainer &commandlines, const std::string &val, Type &outVar)
	{
		outVar = commandlines[val].as<Type>();
	}

	template<class Type, class CommandlineContainer, class Converter>
	void extract(const CommandlineContainer &commandlines, const std::string &val, Type &outVar, Converter &converter)
	{
		outVar = converter(commandlines[val].as<std::string>());
	}

	template<class Type, class CommandlineContainer, class ... MaybeConverter>
	bool extractIfExist(const CommandlineContainer &commandlines, const std::string &val, Type &outVar, MaybeConverter ... converter)
	{
		if (commandlines.count(val))
		{
			extract<Type>(commandlines, val, outVar, converter...);
			return true;
		}

		return false;
	}

	StormExporter::ExportType parseExportType(std::string val)
	{
		boost::to_lower(val);

#define STORMEXPORTER_IF_RETURN(EnumVal, Text) if (val == Text) return StormExporter::ExportType::EnumVal
		STORMEXPORTER_IF_RETURN(Partio, "partio");
#undef STORMEXPORTER_IF_RETURN

		Storm::throwException<Storm::Exception>("Unhandled export type! Value was " + val);
	}

	StormExporter::ExportMode parseExportMode(std::string val)
	{
		boost::to_lower(val);

		StormExporter::ExportMode result = StormExporter::ExportMode::None;

		std::vector<std::string_view> splitted;
		Storm::StringAlgo::split(splitted, val, Storm::StringAlgo::makeSplitPredicate('|', ' ', '\t'));

		for (const auto &command : splitted)
		{
			if (command == "fluid")
			{
				STORM_ADD_BIT_ENABLED(result, StormExporter::ExportMode::Fluid);
			}
			else if (command == "rigidbody")
			{
				STORM_ADD_BIT_ENABLED(result, StormExporter::ExportMode::RigidBody);
			}
			else
			{
				Storm::throwException<Storm::Exception>("Unhandled export mode flag. Value was " + std::string{ command });
			}
		}

		if (result != StormExporter::ExportMode::None)
		{
			return result;
		}
		else
		{
			Storm::throwException<Storm::Exception>("We must set at least a flag to the export mode!");
		}
	}
}


StormExporter::ExporterConfigManager::ExporterConfigManager() :
	_exportMode{ StormExporter::ExportMode::None },
	_exportType{ StormExporter::ExportType::None }
{

}

StormExporter::ExporterConfigManager::~ExporterConfigManager()
{
	this->printHelpAndShouldExit();
}

void StormExporter::ExporterConfigManager::initialize_Implementation(int argc, const char *const argv[])
{
	_desc = std::make_unique<boost::program_options::options_description>("Options");
	_desc->add_options()
		("help,h", "Command line help")
		("mode", boost::program_options::value<std::string>(), "Mode (case insensitive) to export. Combine values with | symbol. Accepted values are : 'Fluid' and 'RigidBodies'.")
		("type", boost::program_options::value<std::string>(), "Type (case insensitive) to export into. Accepted values are : 'Partio'.")
		("in", boost::program_options::value<std::string>(), "The record file path.")
		("out", boost::program_options::value<std::string>(), "The outputted export file path.")
		;

	boost::program_options::variables_map commandlineMap;
	boost::program_options::store(
		boost::program_options::command_line_parser{ argc, argv }
		.options(*_desc)
		.style(boost::program_options::command_line_style::unix_style | boost::program_options::command_line_style::case_insensitive).run(),
		commandlineMap
	);

	boost::program_options::notify(commandlineMap);

	const bool needHelp = commandlineMap.count("help");
	if (!needHelp)
	{
		if (extractIfExist(commandlineMap, "in", _recordToExport))
		{
			if (!std::filesystem::is_regular_file(_recordToExport))
			{
				Storm::throwException<Storm::Exception>(_recordToExport + " is not a file, we cannot export it!");
			}
		}
		else
		{
			Storm::throwException<Storm::Exception>("File to export was not specified!");
		}

		if (!extractIfExist(commandlineMap, "out", _exportPath))
		{
			const std::filesystem::path inRecordPath{ _recordToExport };
			const auto stormRoot = Storm::StormPathHelper::findStormRootPath(std::filesystem::path{ argv[0] }.parent_path());
			
			auto pathToExportOutput = stormRoot / "Intermediate" / "Exporter" / inRecordPath.parent_path().stem() / inRecordPath.filename();
			pathToExportOutput.replace_extension("pdb");

			std::filesystem::create_directories(pathToExportOutput.parent_path());
			
			_exportPath = pathToExportOutput.string();
		}

		if (!extractIfExist(commandlineMap, "mode", _exportMode, parseExportMode))
		{
			Storm::throwException<Storm::Exception>("Export mode was not specified!");
		}

		if (!extractIfExist(commandlineMap, "type", _exportType, parseExportType))
		{
			Storm::throwException<Storm::Exception>("Export type was not specified!");
		}

		_desc.reset();
	}
}

bool StormExporter::ExporterConfigManager::printHelpAndShouldExit()
{
	if (_desc)
	{
		LOG_ALWAYS << *_desc;
		_desc.reset();
		return true;
	}

	return false;
}

const std::string& StormExporter::ExporterConfigManager::getRecordToExport() const
{
	return _recordToExport;
}

const std::string& StormExporter::ExporterConfigManager::getOutExportPath() const
{
	return _exportPath;
}

StormExporter::ExportMode StormExporter::ExporterConfigManager::getExportMode() const
{
	return _exportMode;
}

StormExporter::ExportType StormExporter::ExporterConfigManager::getExportType() const
{
	return _exportType;
}
