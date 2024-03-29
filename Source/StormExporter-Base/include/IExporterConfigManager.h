#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace StormExporter
{
	enum class ExportType : uint8_t;
	enum class ExportMode;

	class IExporterConfigManager : public Storm::ISingletonHeldInterface<IExporterConfigManager>
	{
	protected:
		~IExporterConfigManager() = default;

	public:
		virtual const std::string& getRecordToExport() const = 0;
		virtual const std::string& getOutExportPath() const = 0;
		virtual ExportMode getExportMode() const = 0;
		virtual ExportType getExportType() const = 0;
		virtual std::size_t getSliceOutFrames() const = 0;
	};
}
