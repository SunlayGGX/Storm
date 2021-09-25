#include "FPSWatcher.h"
#include "InvertPeriod.h"


void Storm::FPSWatcher::registerCurrent(const std::chrono::microseconds expectedRefreshTime)
{
	std::chrono::nanoseconds bufferizedTime = _timeBuffer[1] - _timeBuffer[0];
	_timeBuffer[0] = _timeBuffer[1];

	bufferizedTime += _timeBuffer[2] - _timeBuffer[1];
	_timeBuffer[1] = _timeBuffer[2];

	bufferizedTime += _timeBuffer[3] - _timeBuffer[2];
	_timeBuffer[2] = _timeBuffer[3];

	bufferizedTime += _timeBuffer[4] - _timeBuffer[3];
	_timeBuffer[3] = _timeBuffer[4];

	_timeBuffer[4] = std::chrono::high_resolution_clock::now();
	bufferizedTime += _timeBuffer[4] - _timeBuffer[3];

	_cachedFPS = Storm::ChronoHelper::toFps<5>(bufferizedTime);

	_expectedRefreshTime = expectedRefreshTime;
}

float Storm::FPSWatcher::getFps() const
{
	return _cachedFPS;
}

float Storm::FPSWatcher::getExpectedFps() const
{
	return Storm::ChronoHelper::toFps(_expectedRefreshTime);
}

