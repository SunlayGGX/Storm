#include "SpeedProfileData.h"

#include "SingletonHolder.h"
#include "ITimeManager.h"



void Storm::SpeedProfileData::startTime()
{
	_lastStartTime = std::chrono::high_resolution_clock::now();
}

void Storm::SpeedProfileData::stopTime()
{
	using MillisecondFloatDuration = std::chrono::duration<float, std::chrono::milliseconds::period>;

	const MillisecondFloatDuration elapsedRealTime = std::chrono::duration_cast<MillisecondFloatDuration>(std::chrono::high_resolution_clock::now() - _lastStartTime);

	const Storm::ITimeManager &timeMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>();
	const float elapsedVirtualPhysicsTimeMillisec = timeMgr.getCurrentPhysicsDeltaTime() * 1000.f;

	_averageComputationSpeed.addValue(elapsedVirtualPhysicsTimeMillisec / elapsedRealTime.count());
}

const float& Storm::SpeedProfileData::computationSpeed() const
{
	return _averageComputationSpeed.getAverage();
}
