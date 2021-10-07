#include "MemoryWatcher.h"

#include "SingletonHolder.h"
#include "IOSManager.h"

#include "GeneralSafetyConfig.h"

#include "MemoryInfos.h"

#include "SafetyHelpers.h"


Storm::MemoryWatcher::MemoryWatcher(const Storm::GeneralSafetyConfig &safetyConfig) :
	_alertCoeffThreshold{ safetyConfig._memoryThreshold * 0.7 },
	_endCoeffThreshold{ safetyConfig._memoryThreshold },
	_closeRequested{ Storm::MemoryWatcher::CloseRequestLevel::None },
	_alertLogged{ false }
{
	const Storm::IOSManager &osMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IOSManager>();
	_startConsumedMemory = osMgr.retrieveCurrentAppUsedMemory();
}

void Storm::MemoryWatcher::execute()
{
	switch (_closeRequested)
	{
	case Storm::MemoryWatcher::CloseRequestLevel::None:
	{
		const Storm::IOSManager &osMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IOSManager>();
		Storm::MemoryInfos memoryInfos;
		if (osMgr.retrieveMemoryInfo(memoryInfos))
		{
			double memoryConsumptionPercentage = static_cast<double>(memoryInfos._usedMemory) / static_cast<double>(memoryInfos._totalMemory);

			if (memoryConsumptionPercentage > _endCoeffThreshold)
			{
				Storm::SafetyHelpers::sendCleanCloseTermination(
					"We went beyond the alloted memory consumption for the current process. We'll close it"
				);

				_closeTimeRequest = std::chrono::high_resolution_clock::now();
				_closeRequested = Storm::MemoryWatcher::CloseRequestLevel::Closed;
			}
			else if (_alertLogged)
			{
				if (memoryConsumptionPercentage <= _alertCoeffThreshold)
				{
					LOG_DEBUG << "Memory level reduced under " << _alertCoeffThreshold * 100.0 << "%.";
					_alertLogged = false;
				}
			}
			else
			{
				if (memoryConsumptionPercentage > _alertCoeffThreshold)
				{
					std::size_t remainingBeforeKill = static_cast<std::size_t>((memoryConsumptionPercentage - _endCoeffThreshold) * static_cast<double>(memoryInfos._totalMemory));

					LOG_WARNING << "Memory level has gone beyond " << _alertCoeffThreshold * 100.0 << "% of the total ram. If it goes up by " << remainingBeforeKill << " bytes, then we'll kill the application.";
					_alertLogged = true;
				}
			}
		}
		break;
	}
	case Storm::MemoryWatcher::CloseRequestLevel::Closed:
		if (std::chrono::high_resolution_clock::now() - _closeTimeRequest > std::chrono::seconds{ 150 }) // 2.5 Minutes to clean everything seems ok
		{
			Storm::SafetyHelpers::sendCleanExitTermination(
				"Memory watcher already requested a close after 2 minutes 30, nothing was done so we'll soft kill the app now."
			);
			_closeTimeRequest = std::chrono::high_resolution_clock::now();
			_closeRequested = Storm::MemoryWatcher::CloseRequestLevel::Exit;
		}
		break;
	case Storm::MemoryWatcher::CloseRequestLevel::Exit:
		if (std::chrono::high_resolution_clock::now() - _closeTimeRequest > std::chrono::seconds{ 150 }) // 2.5 Minutes to clean everything seems ok
		{
			Storm::SafetyHelpers::sendHardKillTermination(
				"Memory watcher already requested an exit but after 2 minutes 30, nothing was done so we won't joke anymore. We'll hard kill the app now."
			);
		}
		break;

	default:
		assert(false && "Unknown Request level, we should not come here.");
		__assume(false);
	}

}
