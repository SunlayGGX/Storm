#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IProfilerManager : public Storm::ISingletonHeldInterface<Storm::IProfilerManager>
	{
	public:
		virtual ~IProfilerManager() = default;

	public:
		// Register the current thread to be part as the simulation thread to allow it be speed profiled.
		virtual void registerCurrentThreadAsSimulationThread(const std::wstring_view &profileName) = 0;

	public:
		// Profiling methods

		// Start a speed profiling. This measure the speed of the current thread (useful when we want to measure if we can do real time (the simulation time vs reality time).
		// This method is specific for the Simulator, else it is not useful.
		virtual void startSpeedProfile(const std::wstring_view &profileName) = 0;
		virtual void endSpeedProfile(const std::wstring_view &profileName) = 0;
	};
}
