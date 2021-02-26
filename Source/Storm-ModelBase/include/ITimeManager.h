#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	struct ExpectedFPSTag {};
	struct RealTimeFPSTag {};

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
		virtual Storm::TimeWaitResult waitForTime(std::chrono::microseconds timeToWait) = 0;

		// Ask the current thread to wait for the refresh specified by timeToWait.
		// Return value is : 
		// - TimeWaitResult::Continue if the TimeManager is still alive and the simulation isn't paused,
		// - TimeWaitResult::Pause if the TimeManager is still alive but the simulation is paused
		// - TimeWaitResult::Exit if the TimeManager has exited (therefore it is time to exit).
		virtual Storm::TimeWaitResult waitForTime(std::chrono::milliseconds timeToWait) = 0;


		// Ask the current thread to wait for the next frame. This is a handy function for simple loop iterations
		// No need to parse TimeWaitResult.
		// Return value is false if we should exit, true if we should continue.
		virtual bool waitNextFrameOrExit() = 0;

		// Ask the current thread to wait for the refresh specified by timeToWait. This is a handy function for simple loop iterations
		// No need to parse TimeWaitResult.
		// Return value is false if we should exit, true if we should continue.
		virtual bool waitForTimeOrExit(std::chrono::milliseconds timeToWait) = 0;

		// Get the state of the time loop without being synchronized. But a little wait can be expected since this is queried on a multithreaded context.
		virtual Storm::TimeWaitResult getStateNoSyncWait() const = 0;

		/****************************************************************************/
		/*							Simulation Time									*/
		/*	NOTE : It is not the Physics time but the simulation loop update time.	*/
		/****************************************************************************/

		// Set the frame rate. This is not the physics frame rate but more like the simulation expected frame rate (the main thread run loop frame rate).
		virtual void setExpectedFrameFPS(float fps) = 0;

		// Get the expected frame rate of the caller thread. This is not the physics frame rate but more like the simulation expected frame rate (the main thread run loop frame rate).
		virtual float getExpectedFrameFPS() const = 0;

		// Get the expected frame rate of the caller thread. This is not the physics frame rate but more like the simulation expected frame rate (the main thread run loop frame rate).
		// Note that if the thread doesn't use the TimeManager to manage its loop, its fps would be equal to 0.
		virtual float getCurrentFPS(ExpectedFPSTag) const = 0;

		// Get the real time frame rate of the caller thread. This is not the physics frame rate but more like the simulation expected frame rate (the main thread run loop frame rate).
		// Note that if the thread doesn't use the TimeManager to manage its loop, its fps would be equal to 0.
		virtual float getCurrentFPS(RealTimeFPSTag) const = 0;

		// Get the expected frame rate of the specified thread. This is not the physics frame rate but more like the simulation expected frame rate (the main thread run loop frame rate).
		// Note that if the thread doesn't use the TimeManager to manage its loop, its fps would be equal to 0.
		virtual float getFPS(const std::thread::id &threadId, ExpectedFPSTag) const = 0;

		// Get the real time frame rate of the specified thread. This is not the physics frame rate but more like the simulation expected frame rate (the main thread run loop frame rate).
		// Note that if the thread doesn't use the TimeManager to manage its loop, its fps would be equal to 0.
		virtual float getFPS(const std::thread::id &threadId, RealTimeFPSTag) const = 0;

		// Get the elapsed time since the application is running.
		virtual std::chrono::milliseconds getCurrentSimulationElapsedTime() const = 0;



		/************************************************************************/
		/*							Physics Time                                */
		/************************************************************************/

		// Get the simulation physics time in seconds.
		virtual float getCurrentPhysicsDeltaTime() const = 0;

		// Set the simulation physics time in seconds.
		// Returns if the physics time changed from what was before.
		virtual bool setCurrentPhysicsDeltaTime(float deltaTimeInSeconds) = 0;

		// Get the simulation physics time elapsed in seconds.
		virtual float getCurrentPhysicsElapsedTime() const = 0;

		// Increase the simulation physics time elapsed by the passed value in seconds.
		virtual void increaseCurrentPhysicsElapsedTime(float timeIncreaseInSeconds) = 0;

		// Advance the simulation physics time elapsed by the current physics time value.
		// Returns the current time after advancing
		virtual float advanceCurrentPhysicsElapsedTime() = 0;

		// Manually set the physics elapsed time.
		virtual void setCurrentPhysicsElapsedTime(float physicsElapsedTimeInSeconds) = 0;

		// Manually increase the physics delta time step by a fixed step that could be tweaked.
		virtual void increaseCurrentPhysicsDeltaTime() = 0;

		// Manually decrease the physics delta time step by a fixed step that could be tweaked.
		virtual void decreaseCurrentPhysicsDeltaTime() = 0;
		
		// Manually increase the change step of the physics delta time step.
		virtual void increasePhysicsDeltaTimeStepSize() = 0;

		// Manually decrease the change step of the physics delta time step.
		virtual void decreasePhysicsDeltaTimeStepSize() = 0;

		// Reset the physics elasped time value.
		virtual void resetPhysicsElapsedTime() = 0;


		/************************************************************************/
		/*							TimeManager controls                        */
		/************************************************************************/

		// Get if the simulation is paused.
		virtual bool simulationIsPaused() const = 0;

		// Get if the time manager is running. If it is false, then the TimeManager has exited and the application should quit
		virtual bool isRunning() const = 0;

		// If the simulation is paused, unpause it. If it is unpaused, pause it..
		// Note that it doesn't enforce the pause and it doesn't pause the TimeManager looping process. So the frame will still be fired at each
		// framerate intervals...
		// Return the new pause state...
		virtual bool changeSimulationPauseState() = 0;


		// Exit the TimeManager. This should put an end to the simulation by notifying all waiting threads that are waiting for the TimeManager to leave their loop processing.
		// If everything is developed correctly, The main thread should exit too.
		virtual void quit() = 0;
	};
}
