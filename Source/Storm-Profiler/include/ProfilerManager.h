#pragma once

#include "Singleton.h"
#include "SingletonDefaultImplementation.h"
#include "IProfilerManager.h"


namespace Storm
{
	class SpeedProfileHandler;

	class ProfilerManager :
		private Storm::Singleton<Storm::ProfilerManager, Storm::DefineDefaultCleanupImplementationOnly>,
		public Storm::IProfilerManager
	{
		STORM_DECLARE_SINGLETON(ProfilerManager);

	private:
		void initialize_Implementation();

	public:
		void registerCurrentThreadAsSimulationThread(const std::wstring_view &profileName) final override;

	public:
		// Profiling methods
		void startSpeedProfile(const std::wstring_view &profileName) final override;
		void endSpeedProfile(const std::wstring_view &profileName) final override;
		float getSpeedProfileAccumulatedTime() const final override;

	private:
		bool _speedProfile;
		std::map<std::thread::id, Storm::SpeedProfileHandler> _speedProfileHandlerMap;
	};
}
