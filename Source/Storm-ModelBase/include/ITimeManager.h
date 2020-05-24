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
		virtual Storm::TimeWaitResult waitForTime(std::chrono::milliseconds timeToWait) = 0;


		// Ask the current thread to wait for the next frame. This is a handy function for simple loop iterations
		// No need to parse TimeWaitResult.
		// Return value is false if we should exit, true if we should continue.
		virtual bool waitNextFrameOrExit() = 0;

		// Ask the current thread to wait for the refresh specified by timeToWait. This is a handy function for simple loop iterations
		// No need to parse TimeWaitResult.
		// Return value is false if we should exit, true if we should continue.
		virtual bool waitForTimeOrExit(std::chrono::milliseconds timeToWait) = 0;


		/****************************************************************************/
		/*							Simulation Time									*/
		/*	NOTE : It is not the Physics time but the simulation loop update time.	*/
		/****************************************************************************/

		// Set the frame rate. This is not the physics frame rate but more like the simulation expected frame rate (the main thread run loop frame rate).
		virtual void setExpectedFrameFPS(float fps) = 0;

		// Get the expected frame rate of the caller thread. This is not the physics frame rate but more like the simulation expected frame rate (the main thread run loop frame rate).
		// Note that if the thread doesn't use the TimeManager to manage its loop, its fps would be equal to 0.
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
		virtual void setCurrentPhysicsDeltaTime(float deltaTimeInSeconds) = 0;

		// Get the simulation physics time elapsed in seconds.
		virtual float getCurrentPhysicsElapsedTime() const = 0;

		// Increase the simulation physics time elapsed by the passed value in seconds.
		virtual void increaseCurrentPhysicsElapsedTime(float timeIncreaseInSeconds) = 0;


		/************************************************************************/
		/*							TimeManager controls                        */
		/************************************************************************/

		// Get if the simulation is paused.
		virtual bool simulationIsPaused() const = 0;

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
