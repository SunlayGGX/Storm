#include "ProfilerManager.h"

#include "SpeedProfileHandler.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralDebugConfig.h"


Storm::ProfilerManager::ProfilerManager() :
	_speedProfile{ false }
{}

Storm::ProfilerManager::~ProfilerManager() = default;

void Storm::ProfilerManager::initialize_Implementation()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	_speedProfile = configMgr.getGeneralDebugConfig()._profileSimulationSpeed;
}

void Storm::ProfilerManager::registerCurrentThreadAsSimulationThread(const std::wstring_view &profileName)
{
	if (!_speedProfile)
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

		_speedProfile = configMgr.getGeneralDebugConfig()._profileSimulationSpeed;
	}

	if (_speedProfile)
	{
		_speedProfileHandlerMap.emplace(std::this_thread::get_id(), profileName);
	}
}

void Storm::ProfilerManager::startSpeedProfile(const std::wstring_view &profileName)
{
	if (_speedProfile)
	{
		if (this->isInitialized())
		{
			Storm::SpeedProfileHandler &speedProfileHandler = _speedProfileHandlerMap.find(std::this_thread::get_id())->second;
			speedProfileHandler.addProfileData(profileName);
		}
		else
		{
			LOG_ERROR << "Cannot start speed profiling (" << profileName << ") because ProfilerManager wasn't initialized!";
		}
	}
}

void Storm::ProfilerManager::endSpeedProfile(const std::wstring_view &profileName)
{
	if (_speedProfile)
	{
		if (this->isInitialized())
		{
			Storm::SpeedProfileHandler &speedProfileHandler = _speedProfileHandlerMap.find(std::this_thread::get_id())->second;
			speedProfileHandler.removeProfileData(profileName);
		}
		else
		{
			LOG_ERROR << "Cannot start speed profiling (" << profileName << ") because ProfilerManager wasn't initialized!";
		}
	}
}

float Storm::ProfilerManager::getSpeedProfileAccumulatedTime() const
{
	if (_speedProfile)
	{
		const Storm::SpeedProfileHandler &speedProfileHandler = _speedProfileHandlerMap.find(std::this_thread::get_id())->second;
		return speedProfileHandler.getAccumulatedTime();
	}

	return 0.f;
}

float Storm::ProfilerManager::getCurrentSpeedProfile() const
{
	if (_speedProfile)
	{
		const Storm::SpeedProfileHandler &speedProfileHandler = _speedProfileHandlerMap.find(std::this_thread::get_id())->second;
		return speedProfileHandler.getCurrentSpeed();
	}

	return 0.f;
}
