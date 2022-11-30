#include "PartioExporterManager.h"

#include "IExporterConfigManager.h"
#include "ISerializerManager.h"
#include "SingletonHolder.h"

#include "PartioWriter.h"

#include "ExporterEventCallbacks.h"

#include "ExitCode.h"



StormExporter::PartioExporterManager::PartioExporterManager() = default;
StormExporter::PartioExporterManager::~PartioExporterManager() = default;

void StormExporter::PartioExporterManager::initialize_Implementation()
{

}

void StormExporter::PartioExporterManager::doInitialize()
{
	this->initialize();
}

void StormExporter::PartioExporterManager::doCleanUp()
{
	this->cleanUp();
}

Storm::ExitCode StormExporter::PartioExporterManager::run()
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

bool StormExporter::PartioExporterManager::onStartExport(const Storm::SerializeRecordHeader &header)
{
	_writer = std::make_unique<StormExporter::PartioWriter>(header);
	return true;
}

bool StormExporter::PartioExporterManager::onFrameExport(const Storm::SerializeRecordPendingData &frame)
{
	return _writer->onFrameExport(frame);
}

void StormExporter::PartioExporterManager::onExportClose()
{
	_writer->onExportClose();
	_writer.reset();
}
