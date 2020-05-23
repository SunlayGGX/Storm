#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	enum class TimeWaitResult;

	class ITimeManager : public Storm::ISingletonHeldInterface<ITimeManager>
	{
	public:
		virtual ~ITimeManager() = default;

	public:
		/************************************************************************/
		/*							Synchronizer                                */
		/************************************************************************/

		// Ask the current thread to wait for the next frame.
		// Return value is : 
		// - TimeWaitResult::Continue if the TimeManager is still alive and the simulation isn't paused,
		// - TimeWaitResult::Pause if the TimeManager is still alive but the simulation is paused
		// - TimeWaitResult::Exit if the TimeManager has exited (therefore it is time to exit).
		virtual Storm::TimeWaitResult waitNextFrame() = 0;

		// Ask the current thread to wait for the refresh specified by timeToWait.
		// Return value is : 
		// - TimeWaitResult::Continue if the TimeManager is still alive and the simulation isn't paused,
		// - TimeWaitResult::Pause if the TimeManager is still alive but the simulation is paused
		// - TimeWaitResult::Exit if the TimeManager has exited (therefore it is time to exit).
		virtual Storm::TimeWaitResult waitForTime(std::chrono::milliseconds timeToWait) = 0;


		/************************************************************************/
		/*							Simulation Time                             */
		/************************************************************************/

		// Set the frame rate. This is not the physics frame rate but more like the simulation expected frame rate (the main thread run loop frame rate).
		virtual void setCurrentFPS(float fps) const = 0;

		// Get the frame rate. This is not the physics frame rate but more like the simulation expected frame rate (the main thread run loop frame rate).
		virtual float getCurrentFPS() const = 0;

		// Get the elapsed time since the application is running.
		virtual std::chrono::milliseconds getCurrentSimulationElapsedTime() const = 0;



		/************************************************************************/
		/*							Physics Time                                */
		/************************************************************************/

		// Get the simulation physics time in seconds.
		virtual float getCurrentPhysicsDeltaTime() const = 0;

		// Set the simulation physics time in seconds.
		virtual void setCurrentPhysicsDeltaTime(float deltaTimeInSeconds) = 0;

		// Get the simulation physics time elapsed in seconds.
		virtual float getCurrentPhysicsElapsedTime() const = 0;


		/************************************************************************/
		/*							TimeManager controls                        */
		/************************************************************************/

		// Exit the TimeManager. This should put an end to the simulation by notifying all waiting threads that are waiting for the TimeManager to leave their loop processing.
		// If everything is developed correctly, The main thread should exit too.
		virtual void quit() = 0;
	};
}
