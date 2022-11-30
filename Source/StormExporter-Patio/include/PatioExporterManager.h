#pragma once

#include "Singleton.h"
#include "IExporterManager.h"
#include "SingletonDefaultImplementation.h"

namespace Storm
{
	struct SerializeRecordHeader;
	struct SerializeRecordPendingData;
}

namespace StormExporter
{
	class PatioExporterManager final :
		private Storm::Singleton<PatioExporterManager, Storm::DefineDefaultCleanupImplementationOnly>,
		public StormExporter::IExporterManager
	{
		STORM_DECLARE_SINGLETON(PatioExporterManager);

	private:
		void initialize_Implementation();

	public:
		void doInitialize() final override;
		void doCleanUp() final override;
		Storm::ExitCode run() final override;

	private:
		bool onStartExport(const Storm::SerializeRecordHeader &header);
		bool onFrameExport(const Storm::SerializeRecordPendingData &frame);
		void onExportClose();
	};
}
