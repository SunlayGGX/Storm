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
		virtual ExportMode getExportMode() const = 0;
		virtual ExportType getExportType() const = 0;
	};
}
