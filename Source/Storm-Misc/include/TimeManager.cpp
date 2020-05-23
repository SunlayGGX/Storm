#include "TimeManager.h"
#include "SerializePackage.h"


Storm::TimeManager::TimeManager() :
	_physicsTimeInSeconds{ 0.05f },
	_startTime{ std::chrono::high_resolution_clock::now() },
	_isRunning{ false }
{

}

Storm::TimeManager::~TimeManager() = default;

void Storm::TimeManager::initialize_Implementation()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void Storm::TimeManager::cleanUp_Implementation()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void Storm::TimeManager::serialize(Storm::SerializePackage &packet)
{
	packet 
		<< _physicsTimeInSeconds
		<< _physicsElapsedTimeInSeconds
		;
}

Storm::TimeWaitResult Storm::TimeManager::waitNextFrame()
{
	throw std::logic_error("The method or operation is not implemented.");
}

Storm::TimeWaitResult Storm::TimeManager::waitForTime(std::chrono::milliseconds timeToWait)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void Storm::TimeManager::setCurrentFPS(float fps) const
{
	throw std::logic_error("The method or operation is not implemented.");
}

float Storm::TimeManager::getCurrentFPS() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

std::chrono::milliseconds Storm::TimeManager::getCurrentSimulationElapsedTime() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

float Storm::TimeManager::getCurrentPhysicsDeltaTime() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

void Storm::TimeManager::setCurrentPhysicsDeltaTime(float deltaTimeInSeconds)
{
	throw std::logic_error("The method or operation is not implemented.");
}

float Storm::TimeManager::getCurrentPhysicsElapsedTime() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

void Storm::TimeManager::quit()
{
	throw std::logic_error("The method or operation is not implemented.");
}
