#include "FPSWatcher.h"
#include "InvertPeriod.h"


void Storm::FPSWatcher::registerCurrent(std::chrono::microseconds expectedRefreshTime)
{
	std::chrono::nanoseconds _bufferizedTime = _timeBuffer[1] - _timeBuffer[0];
	_timeBuffer[0] = _timeBuffer[1];

	_bufferizedTime = _timeBuffer[2] - _timeBuffer[1];
	_timeBuffer[1] = _timeBuffer[2];

	_bufferizedTime = _timeBuffer[3] - _timeBuffer[2];
	_timeBuffer[2] = _timeBuffer[3];

	_bufferizedTime = _timeBuffer[4] - _timeBuffer[3];
	_timeBuffer[3] = _timeBuffer[4];

	_bufferizedTime = _timeBuffer[5] - _timeBuffer[4];
	_timeBuffer[4] = _timeBuffer[5];

	_timeBuffer[5] = std::chrono::high_resolution_clock::now();
	_bufferizedTime = _timeBuffer[5] - _timeBuffer[4];

	constexpr float coefficientBufferNanoToFps = Storm::InvertPeriod<decltype(_bufferizedTime)::period>::value * 6.f;
	_cachedFPS = coefficientBufferNanoToFps / static_cast<float>(_bufferizedTime.count());

	_expectedRefreshTime = expectedRefreshTime;
}

float Storm::FPSWatcher::getFps() const
{
	return _cachedFPS;
}

float Storm::FPSWatcher::getExpectedFps() const
{
	constexpr float coefficientMicroToFps = Storm::InvertPeriod<decltype(_expectedRefreshTime)::period>::value;
	return coefficientMicroToFps / static_cast<float>(_expectedRefreshTime.count());
}

