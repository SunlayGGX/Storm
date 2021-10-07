#pragma once


namespace Storm
{
	struct GeneralSafetyConfig;

	class FreezeWatcher
	{
	public:
		using TimePoint = std::chrono::high_resolution_clock::time_point;

	public:
		FreezeWatcher(const Storm::GeneralSafetyConfig &safetyConfig);

	public:
		void execute();

	public:
		void setLastNotificationTime(const TimePoint lastNotifTime);

	private:
		void resetFlags();

	private:
		TimePoint _lastNotifTime;
		std::pair<const std::chrono::seconds, bool> _timeBeforeHardKill;
		std::pair<const std::chrono::seconds, bool> _timeBeforeSoftKill;
		std::pair<const std::chrono::seconds, bool> _timeSoftQuit;
		std::pair<const std::chrono::seconds, bool> _alertTime;

		bool _lastDebuggerAttachedState;
	};
}
