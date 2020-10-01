#include "SpeedProfileData.h"

#include "MemoryHelper.h"

#include "SingletonHolder.h"
#include "ITimeManager.h"



Storm::SpeedProfileData::SpeedProfileData() :
	_currentComputationSpeed{ 0.f }
{
	_bufferPtr = _lastSpeedBuffer;
	std::fill(std::begin(_lastSpeedBuffer), std::end(_lastSpeedBuffer), 0.f);
}

void Storm::SpeedProfileData::startTime()
{
	_lastStartTime = std::chrono::high_resolution_clock::now();
}

void Storm::SpeedProfileData::stopTime()
{
	using MillisecondFloatDuration = std::chrono::duration<float, std::chrono::milliseconds::period>;

	const auto endTime = std::chrono::high_resolution_clock::now();

	const MillisecondFloatDuration elapsedRealTime = std::chrono::duration_cast<MillisecondFloatDuration>(endTime - _lastStartTime);

	const Storm::ITimeManager &timeMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ITimeManager>();
	const float elapsedVirtualPhysicsTimeMillisec = timeMgr.getCurrentPhysicsDeltaTime() * 1000.f;

	*_bufferPtr = elapsedVirtualPhysicsTimeMillisec / elapsedRealTime.count();

	_bufferPtr = _lastSpeedBuffer + ((_bufferPtr - _lastSpeedBuffer) + 1) % Storm::SpeedProfileData::k_speedBufferCount;

	for (const float registeredTime : _lastSpeedBuffer)
	{
		_currentComputationSpeed += registeredTime;
	}

	_currentComputationSpeed /= static_cast<float>(Storm::SpeedProfileData::k_speedBufferCount);
}

const float& Storm::SpeedProfileData::computationSpeed() const
{
	return _currentComputationSpeed;
}
