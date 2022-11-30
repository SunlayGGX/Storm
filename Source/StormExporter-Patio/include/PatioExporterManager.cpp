#include "PatioExporterManager.h"
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
	return Storm::ExitCode::k_success;
}
