#include "PatioExporterManager.h"

#include "IExporterConfigManager.h"
#include "ISerializerManager.h"
#include "SingletonHolder.h"

#include "ExporterEventCallbacks.h"

#include "ExitCode.h"



StormExporter::PatioExporterManager::PatioExporterManager() = default;
StormExporter::PatioExporterManager::~PatioExporterManager() = default;

void StormExporter::PatioExporterManager::initialize_Implementation()
{

}

void StormExporter::PatioExporterManager::doInitialize()
{
	this->initialize();
}

void StormExporter::PatioExporterManager::doCleanUp()
{
	this->cleanUp();
}

Storm::ExitCode StormExporter::PatioExporterManager::run()
{
	const auto &singletonHolder = Storm::SingletonHolder::instance();
	const auto &configMgr = singletonHolder.getSingleton<StormExporter::IExporterConfigManager>();
	auto &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();

	serializerMgr.exportRecord(configMgr.getRecordToExport(), Storm::ExporterEventCallbacks{
		._onStartRecordRead = [this](const auto &header) { return this->onStartExport(header); },
		._onNewFrameReceive = [this](const auto &frame) { return this->onFrameExport(frame); },
		._onRecordClose = [this]() { this->onExportClose(); }
	});

	return Storm::ExitCode::k_success;
}

bool StormExporter::PatioExporterManager::onStartExport(const Storm::SerializeRecordHeader &header)
{
	return true;
}

bool StormExporter::PatioExporterManager::onFrameExport(const Storm::SerializeRecordPendingData &frame)
{
	return true;
}

void StormExporter::PatioExporterManager::onExportClose()
{

}
