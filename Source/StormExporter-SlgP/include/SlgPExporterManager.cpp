#include "SlgPExporterManager.h"

#include "IExporterConfigManager.h"
#include "ISerializerManager.h"
#include "SingletonHolder.h"

#include "SlgPWriter.h"

#include "ExporterEventCallbacks.h"

#include "ExitCode.h"



StormExporter::SlgPExporterManager::SlgPExporterManager() = default;
StormExporter::SlgPExporterManager::~SlgPExporterManager() = default;

void StormExporter::SlgPExporterManager::initialize_Implementation()
{

}

void StormExporter::SlgPExporterManager::doInitialize()
{
	this->initialize();
}

void StormExporter::SlgPExporterManager::doCleanUp()
{
	this->cleanUp();
}

Storm::ExitCode StormExporter::SlgPExporterManager::run()
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

bool StormExporter::SlgPExporterManager::onStartExport(const Storm::SerializeRecordHeader &header)
{
	_writer = std::make_unique<StormExporter::SlgPWriter>(header);
	return true;
}

bool StormExporter::SlgPExporterManager::onFrameExport(const Storm::SerializeRecordPendingData &frame)
{
	return _writer->onFrameExport(frame);
}

void StormExporter::SlgPExporterManager::onExportClose()
{
	_writer->onExportClose();
	_writer.reset();
}
