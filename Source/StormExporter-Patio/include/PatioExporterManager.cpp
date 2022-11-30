#include "PatioExporterManager.h"

#include "IExporterConfigManager.h"
#include "ISerializerManager.h"
#include "SingletonHolder.h"

#include "PatioWriter.h"

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
	_writer = std::make_unique<StormExporter::PatioWriter>(header);
	return true;
}

bool StormExporter::PatioExporterManager::onFrameExport(const Storm::SerializeRecordPendingData &frame)
{
	return _writer->onFrameExport(frame);
}

void StormExporter::PatioExporterManager::onExportClose()
{
	_writer->onExportClose();
	_writer.reset();
}
