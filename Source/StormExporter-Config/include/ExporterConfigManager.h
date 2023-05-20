#pragma once

#include "Singleton.h"
#include "IExporterConfigManager.h"
#include "SingletonDefaultImplementation.h"

namespace boost
{
	namespace program_options
	{
		class options_description;
	}
}

namespace StormExporter
{
	class ExporterConfigManager final :
		private Storm::Singleton<ExporterConfigManager, Storm::DefineDefaultCleanupImplementationOnly>,
		public StormExporter::IExporterConfigManager
	{
		STORM_DECLARE_SINGLETON(ExporterConfigManager);

	public:
		void initialize_Implementation(int argc, const char *const argv[]);

	public:
		bool printHelpAndShouldExit();

	public:
		const std::string& getRecordToExport() const final override;
		const std::string& getOutExportPath() const final override;
		ExportMode getExportMode() const final override;
		ExportType getExportType() const final override;
		std::size_t getSliceOutFrames() const final override;

	private:
		ExportMode _exportMode;
		ExportType _exportType;
		std::string _recordToExport;
		std::string _exportPath;

		std::size_t _sliceOutFrames;

		std::unique_ptr<boost::program_options::options_description> _desc;
	};
}
