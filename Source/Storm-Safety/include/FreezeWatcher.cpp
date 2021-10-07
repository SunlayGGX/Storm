#include "FreezeWatcher.h"

#include "GeneralSafetyConfig.h"

#include "SafetyHelpers.h"

#include "ThreadingSafety.h"


Storm::FreezeWatcher::FreezeWatcher(const Storm::GeneralSafetyConfig &safetyConfig) :
	_lastNotifTime{ std::chrono::high_resolution_clock::now() },
	_timeBeforeHardKill{ safetyConfig._freezeRefresh * 2, false },
	_timeBeforeSoftKill{ static_cast<long long>(safetyConfig._freezeRefresh.count() * 1.5f) + 1, false },
	_timeSoftQuit{ safetyConfig._freezeRefresh, false },
	_alertTime{ safetyConfig._freezeRefresh / 2, false },
	_lastDebuggerAttachedState{ Storm::isDebuggerAttached() }
{
	
}

void Storm::FreezeWatcher::execute()
{
	const auto diffTimeNotifNow = std::chrono::high_resolution_clock::now() - _lastNotifTime;
	const bool debuggerAttached = Storm::isDebuggerAttached();

	// If we detached the debugger.
	if (_lastDebuggerAttachedState != debuggerAttached) STORM_UNLIKELY
	{
		_lastDebuggerAttachedState = debuggerAttached;
		this->resetFlags();
	}

	if (diffTimeNotifNow > _timeBeforeHardKill.first) STORM_UNLIKELY
	{
		if (debuggerAttached)
		{
			if (!_timeBeforeHardKill.second)
			{
				LOG_WARNING <<
					"We went beyond the hard kill freeze duration threshold but a debugger is attached. Maybe this was because we're stepping into the code therefore no killing will be done.\n"
					"If you want to reenable freeze watching, unattach the debugger.";
				_timeBeforeHardKill.second = true;
			}
		}
		else if (!_timeBeforeHardKill.second)
		{
			Storm::SafetyHelpers::sendHardKillTermination(
				"We went beyond the hard kill freeze duration threshold. Now this is no time to joke, we're hard killing the application!"
			);
		}
	}
	else if (diffTimeNotifNow > _timeBeforeSoftKill.first) STORM_UNLIKELY
	{
		if (debuggerAttached)
		{
			if (!_timeBeforeSoftKill.second)
			{
				LOG_WARNING <<
					"We went beyond the soft kill freeze duration threshold but a debugger is attached. Maybe this was because we're stepping into the code therefore no killing will be done.\n"
					"If you want to reenable freeze watching, unattach the debugger.";
				_timeBeforeSoftKill.second = true;
			}
		}
		else if (!_timeBeforeSoftKill.second)
		{
			Storm::SafetyHelpers::sendCleanExitTermination(
				"We went beyond the soft kill freeze duration threshold. So we cannot quit cleanly, therefore we're switching to killing! At first, we'll be soft with a clean kill."
			);
		}
	}
	else if (diffTimeNotifNow > _timeSoftQuit.first) STORM_UNLIKELY
	{
		if (debuggerAttached)
		{
			if (!_timeSoftQuit.second)
			{
				LOG_WARNING <<
					"We went beyond the soft quit freeze duration threshold but a debugger is attached. Maybe this was because we're stepping into the code therefore no quitting will be done.\n"
					"If you want to reenable freeze watching, unattach the debugger.";
				_timeSoftQuit.second = true;
			}
		}
		else if (!_timeSoftQuit.second)
		{
			Storm::SafetyHelpers::sendCleanCloseTermination(
				"We went beyond the soft quit freeze duration threshold. Therefore we consider the simulator as frozen, we'll quit the application naturally."
			);
		}
	}
	else if (diffTimeNotifNow > _alertTime.first) STORM_UNLIKELY
	{
		if (debuggerAttached)
		{
			if (!_alertTime.second)
			{
				LOG_WARNING << "Debugger is attached but we get to warn user nevertheless of a possible freeze happening. But as long as the debugger is attached, the killing won't be done.";
				_alertTime.second = true;
			}
		}
		else if (!_alertTime.second)
		{
			LOG_WARNING <<
				"Either the freeze threshold is too low or a real freeze is happening.\n"
				"Nevertheless, if the simulator is in the same state for ";
		}
	}
	else
	{
		this->resetFlags();
	}
}

void Storm::FreezeWatcher::setLastNotificationTime(const Storm::FreezeWatcher::TimePoint lastNotifTime)
{
	assert(Storm::isSafetyThread() && "This method can only be executed inside Safety thread.");
	_lastNotifTime = lastNotifTime;
}

void Storm::FreezeWatcher::resetFlags()
{
	_timeBeforeHardKill.second = false;
	_timeBeforeSoftKill.second = false;
	_timeSoftQuit.second = false;
	_alertTime.second = false;
}
